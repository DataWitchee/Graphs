#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <json/json.h>

using namespace std;

static const double EPS = 1e-9;
static const double BATTERY_CAPACITY = 500.0;
static const double CHARGE_RATE = 2.0;
static const size_t SEED_SEARCH_LIMIT = 180;
static const size_t EXT_SEARCH_LIMIT = 250;

struct Point {
    double x = 0.0;
    double y = 0.0;
};

struct Delivery {
    string id;
    double x = 0.0;
    double y = 0.0;
    double weight = 0.0;
    double deadline = 0.0;
    bool assigned = false;
};

struct ChargingStation {
    double x = 0.0;
    double y = 0.0;
    int slots = 1;
};

struct NFZ {
    string shape;
    Point center;
    double radius = 0.0;
    Point minCorner;
    Point maxCorner;
    double tStart = 0.0;
    double tEnd = 0.0;
};

struct DroneState {
    string id;
    double maxPayload = 0.0;
    double availableTime = 0.0;
    double battery = BATTERY_CAPACITY;
    vector<Json::Value> steps;
};

struct TravelCheck {
    bool feasible = false;
    double startTime = 0.0;
    double endTime = 0.0;
    double batteryAfter = 0.0;
    double distance = 0.0;
};

static inline double sqr(double v) { return v * v; }

static inline double dist(const Point &a, const Point &b) {
    return hypot(a.x - b.x, a.y - b.y);
}

static inline bool samePoint(const Point &a, const Point &b) {
    return fabs(a.x - b.x) <= EPS && fabs(a.y - b.y) <= EPS;
}

static inline Json::Value makeStep(double x, double y, double t, const string &action) {
    Json::Value v(Json::objectValue);
    v["x"] = x;
    v["y"] = y;
    v["t"] = t;
    v["action"] = action;
    return v;
}

static inline Json::Value makePickupStep(const Point &p, double t, const vector<string> &ids) {
    Json::Value v = makeStep(p.x, p.y, t, "PICKUP");
    Json::Value arr(Json::arrayValue);
    for (const string &id : ids) arr.append(id);
    v["delivery_ids"] = arr;
    return v;
}

static inline Json::Value makeDeliverStep(const Point &p, double t, const string &id) {
    Json::Value v = makeStep(p.x, p.y, t, "DELIVER");
    v["delivery_id"] = id;
    return v;
}

static bool segmentCircleInterval(const Point &a, const Point &b, const NFZ &z, double &u0, double &u1) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double fx = a.x - z.center.x;
    double fy = a.y - z.center.y;

    double A = dx * dx + dy * dy;
    if (A <= EPS) {
        double d2 = sqr(a.x - z.center.x) + sqr(a.y - z.center.y);
        if (d2 <= sqr(z.radius) + EPS) {
            u0 = 0.0;
            u1 = 1.0;
            return true;
        }
        return false;
    }

    double B = 2.0 * (fx * dx + fy * dy);
    double C = fx * fx + fy * fy - z.radius * z.radius;
    double disc = B * B - 4.0 * A * C;
    if (disc < -EPS) return false;
    disc = max(0.0, disc);
    double s = sqrt(disc);
    double t1 = (-B - s) / (2.0 * A);
    double t2 = (-B + s) / (2.0 * A);
    if (t1 > t2) swap(t1, t2);
    u0 = max(0.0, t1);
    u1 = min(1.0, t2);
    return u0 <= u1 + EPS;
}

static bool segmentRectInterval(const Point &a, const Point &b, const NFZ &z, double &u0, double &u1) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;

    double tEnter = 0.0, tExit = 1.0;

    auto clip = [&](double p, double q) -> bool {
        if (fabs(p) <= EPS) {
            return q >= -EPS;
        }
        double r = q / p;
        if (p < 0.0) {
            if (r > tExit + EPS) return false;
            if (r > tEnter) tEnter = r;
        } else {
            if (r < tEnter - EPS) return false;
            if (r < tExit) tExit = r;
        }
        return true;
    };

    if (!clip(-dx, a.x - z.minCorner.x)) return false;
    if (!clip(dx, z.maxCorner.x - a.x)) return false;
    if (!clip(-dy, a.y - z.minCorner.y)) return false;
    if (!clip(dy, z.maxCorner.y - a.y)) return false;

    if (tEnter > tExit + EPS) return false;
    u0 = max(0.0, tEnter);
    u1 = min(1.0, tExit);
    return u0 <= u1 + EPS;
}

static bool segmentIntersectionInterval(const Point &a, const Point &b, const NFZ &z, double &u0, double &u1) {
    if (z.shape == "circle") return segmentCircleInterval(a, b, z, u0, u1);
    return segmentRectInterval(a, b, z, u0, u1);
}

static vector<pair<double, double>> forbiddenStartIntervals(const Point &a, const Point &b, const vector<NFZ> &nfzs) {
    vector<pair<double, double>> intervals;
    double d = dist(a, b);
    if (d <= EPS) return intervals;

    for (const auto &z : nfzs) {
        double u0 = 0.0, u1 = 0.0;
        if (!segmentIntersectionInterval(a, b, z, u0, u1)) continue;
        double start = z.tStart - u1 * d;
        double end = z.tEnd - u0 * d;
        if (end + EPS < start) continue;
        intervals.push_back({start, end});
    }

    sort(intervals.begin(), intervals.end());
    return intervals;
}

static double safeStartTime(double currentTime, const Point &a, const Point &b, const vector<NFZ> &nfzs) {
    double t = currentTime;
    double d = dist(a, b);
    if (d <= EPS) return t;

    vector<pair<double, double>> intervals = forbiddenStartIntervals(a, b, nfzs);
    if (intervals.empty()) return t;

    for (int iter = 0; iter < 8; ++iter) {
        double nextT = t;
        for (const auto &iv : intervals) {
            if (t + EPS >= iv.first && t <= iv.second + EPS) {
                nextT = max(nextT, iv.second + EPS);
            }
        }
        if (nextT <= t + EPS) break;
        t = nextT;
    }
    return t;
}

static TravelCheck checkTravel(const Point &a, const Point &b, double currentTime, double battery, double payload, const vector<NFZ> &nfzs) {
    TravelCheck res;
    res.distance = dist(a, b);
    res.startTime = safeStartTime(currentTime, a, b, nfzs);
    res.endTime = res.startTime + res.distance;
    double energy = res.distance * (1.0 + payload);
    if (battery + EPS < energy) return res;
    res.batteryAfter = battery - energy;
    res.feasible = true;
    return res;
}

static int findStationIndex(const Point &p, const vector<ChargingStation> &stations) {
    for (int i = 0; i < (int)stations.size(); ++i) {
        if (fabs(stations[i].x - p.x) <= EPS && fabs(stations[i].y - p.y) <= EPS) return i;
    }
    return -1;
}

static double costToWarehouse(const Point &from, double currentTime, const vector<NFZ> &nfzs, double payload = 0.0) {
    Point warehouse;
    warehouse.x = 0.0;
    warehouse.y = 0.0;
    (void)currentTime;
    (void)nfzs;
    (void)payload;
    return 0.0;
}

static bool canReturnDirectly(const Point &from, double currentTime, double battery, const vector<NFZ> &nfzs, double payload = 0.0) {
    Point warehouse;
    warehouse.x = 0.0;
    warehouse.y = 0.0;
    TravelCheck ret = checkTravel(from, warehouse, currentTime, battery, payload, nfzs);
    return ret.feasible;
}

static TravelCheck travelToWarehouse(const Point &from, double currentTime, double battery, const vector<NFZ> &nfzs, double payload = 0.0) {
    Point warehouse;
    warehouse.x = 0.0;
    warehouse.y = 0.0;
    return checkTravel(from, warehouse, currentTime, battery, payload, nfzs);
}

static double directCostToWarehouse(const Point &from) {
    Point warehouse;
    warehouse.x = 0.0;
    warehouse.y = 0.0;
    return dist(from, warehouse) * 1.0;
}

static bool isFeasibleReturnState(const Point &pos, double time, double battery, const vector<NFZ> &nfzs, const vector<ChargingStation> &stations) {
    TravelCheck ret = travelToWarehouse(pos, time, battery, nfzs, 0.0);
    if (ret.feasible) return true;
    if (findStationIndex(pos, stations) >= 0) {
        double need = dist(pos, Point{0.0, 0.0}) * 1.0;
        double targetBattery = min(BATTERY_CAPACITY, battery + max(0.0, need - battery));
        (void)targetBattery;
        return true;
    }
    return false;
}

static bool tryChargeAndReturn(Point pos, double &time, double &battery, vector<Json::Value> &steps, const vector<NFZ> &nfzs, const vector<ChargingStation> &stations) {
    int idx = findStationIndex(pos, stations);
    if (idx < 0) return false;

    TravelCheck ret = travelToWarehouse(pos, time, battery, nfzs, 0.0);
    double need = ret.distance;
    if (!ret.feasible) {
        double required = need - battery;
        if (required > BATTERY_CAPACITY - battery + EPS) return false;
        double chargeTime = ceil(max(0.0, required) / CHARGE_RATE - EPS);
        if (chargeTime < 0.0) chargeTime = 0.0;
        if (chargeTime > 0.0) {
            steps.push_back(makeStep(pos.x, pos.y, time, "CHARGE"));
            time += chargeTime;
            battery = min(BATTERY_CAPACITY, battery + chargeTime * CHARGE_RATE);
            steps.push_back(makeStep(pos.x, pos.y, time, "CHARGE_COMPLETE"));
        }
        ret = travelToWarehouse(pos, time, battery, nfzs, 0.0);
        if (!ret.feasible) return false;
    }
    time = ret.endTime;
    battery = ret.batteryAfter;
    steps.push_back(makeStep(0.0, 0.0, time, "RETURN"));
    return true;
}

static void appendWaitIfNeeded(vector<Json::Value> &steps, const Point &p, double fromTime, double toTime) {
    if (toTime > fromTime + EPS) {
        steps.push_back(makeStep(p.x, p.y, toTime, "WAIT"));
    }
}

static bool applyTravel(Point &pos, double &time, double &battery, double payload, const Point &to, vector<Json::Value> &steps, const vector<NFZ> &nfzs) {
    TravelCheck tr = checkTravel(pos, to, time, battery, payload, nfzs);
    if (!tr.feasible) return false;
    appendWaitIfNeeded(steps, pos, time, tr.startTime);
    time = tr.endTime;
    battery = tr.batteryAfter;
    pos = to;
    return true;
}

static vector<int> collectTopUnassigned(const vector<int> &sortedIdx, const vector<Delivery> &deliveries, size_t limit) {
    vector<int> out;
    out.reserve(limit);
    for (int idx : sortedIdx) {
        if (!deliveries[idx].assigned) {
            out.push_back(idx);
            if (out.size() >= limit) break;
        }
    }
    return out;
}

static bool routeStillReturnable(const Point &pos, double time, double battery, const vector<NFZ> &nfzs, const vector<ChargingStation> &stations) {
    TravelCheck ret = travelToWarehouse(pos, time, battery, nfzs, 0.0);
    if (ret.feasible) return true;
    return findStationIndex(pos, stations) >= 0;
}

static bool candidateFeasibleAfterArrival(const Point &pos, double time, double battery, const vector<NFZ> &nfzs, const vector<ChargingStation> &stations) {
    if (routeStillReturnable(pos, time, battery, nfzs, stations)) return true;
    return false;
}

static bool simulateCandidate(const Point &startPos, double startTime, double startBattery, double payloadBefore, const Delivery &cand, const vector<NFZ> &nfzs, const vector<ChargingStation> &stations,
                              Point &outPos, double &outTime, double &outBattery, double &outPayload, vector<Json::Value> *steps = nullptr) {
    Point target{cand.x, cand.y};
    TravelCheck tr = checkTravel(startPos, target, startTime, startBattery, payloadBefore, nfzs);
    if (!tr.feasible) return false;
    if (tr.endTime > cand.deadline + EPS) return false;

    Point pos = startPos;
    double time = startTime;
    double battery = startBattery;
    if (steps) {
        appendWaitIfNeeded(*steps, pos, time, tr.startTime);
    }
    time = tr.endTime;
    battery = tr.batteryAfter;
    pos = target;

    outPos = pos;
    outTime = time;
    outBattery = battery;
    outPayload = max(0.0, payloadBefore - cand.weight);

    return candidateFeasibleAfterArrival(outPos, outTime, outBattery, nfzs, stations);
}

struct PlannedTrip {
    bool feasible = false;
    vector<int> deliveryIdx;
    vector<Json::Value> steps;
    double endTime = 0.0;
    double endBattery = 0.0;
};

static PlannedTrip planTripForDrone(const DroneState &drone, const vector<Delivery> &deliveries, const vector<int> &sortedIdx,
                                    const vector<NFZ> &nfzs, const vector<ChargingStation> &stations) {
    PlannedTrip trip;
    Point warehouse{0.0, 0.0};
    Point pos = warehouse;
    double time = drone.availableTime;
    double battery = drone.battery;
    double payload = 0.0;
    double totalPayload = 0.0;

    vector<int> seedCandidates = collectTopUnassigned(sortedIdx, deliveries, SEED_SEARCH_LIMIT);

    int bestSeed = -1;
    double bestSeedKeyDeadline = numeric_limits<double>::infinity();
    double bestSeedKeyDist = numeric_limits<double>::infinity();

    for (int idx : seedCandidates) {
        const Delivery &cand = deliveries[idx];
        if (cand.assigned) continue;
        if (cand.weight > drone.maxPayload + EPS) continue;

        Point outPos;
        double outTime, outBattery, outPayload;
        if (!simulateCandidate(pos, time, battery, payload + cand.weight, cand, nfzs, stations, outPos, outTime, outBattery, outPayload)) continue;

        double d = dist(pos, Point{cand.x, cand.y});
        if (cand.deadline < bestSeedKeyDeadline - EPS || (fabs(cand.deadline - bestSeedKeyDeadline) <= EPS && d < bestSeedKeyDist - EPS)) {
            bestSeed = idx;
            bestSeedKeyDeadline = cand.deadline;
            bestSeedKeyDist = d;
        }
    }

    if (bestSeed < 0) return trip;

    auto acceptDelivery = [&](int idx) {
        const Delivery &cand = deliveries[idx];
        Point target{cand.x, cand.y};
        TravelCheck tr = checkTravel(pos
