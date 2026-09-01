// Start of HEAD
#include <algorithm>
#include <cmath>
#include <iostream>
#include <json/json.h>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct PR {
  double wx, wy;
  bool hasWP;
  double wait;
  bool ok;
};

int main() {
  string input_str((istreambuf_iterator<char>(cin)),
                   istreambuf_iterator<char>());
  Json::Value input_data;
  Json::CharReaderBuilder rb;
  string errs;
  istringstream ss(input_str);
  Json::parseFromStream(rb, ss, &input_data, &errs);

  double mapW = input_data["map_size"][0].asDouble();
  double mapH = input_data["map_size"][1].asDouble();
  double warehouseX = mapW / 2.0, warehouseY = mapH / 2.0;
  Json::Value drones = input_data["drones"];
  Json::Value deliveries = input_data["deliveries"];
  Json::Value no_fly_zones =
      input_data.get("no_fly_zones", Json::Value(Json::arrayValue));
  Json::Value charging_stations =
      input_data.get("charging_stations", Json::Value(Json::arrayValue));
  // End of HEAD

  // Start of BODY
  int ND = (int)deliveries.size(), NN = (int)no_fly_zones.size(),
      NDRONES = (int)drones.size();
  vector<string> dId(ND);
  vector<double> dX(ND), dY(ND), dW(ND), dDL(ND);
  for (int i = 0; i < ND; i++) {
    dId[i] = deliveries[i]["id"].asString();
    dX[i] = deliveries[i]["x"].asDouble();
    dY[i] = deliveries[i]["y"].asDouble();
    dW[i] = deliveries[i]["weight"].asDouble();
    dDL[i] = deliveries[i]["deadline"].asDouble();
  }
  vector<string> drId(NDRONES);
  vector<double> drMP(NDRONES);
  for (int i = 0; i < NDRONES; i++) {
    drId[i] = drones[i]["id"].asString();
    drMP[i] = drones[i]["max_payload"].asDouble();
  }
  vector<int> nSh(NN);
  vector<double> nCX(NN), nCY(NN), nR(NN), nXN(NN), nYN(NN), nXX(NN), nYX(NN),
      nTS(NN), nTE(NN);
  double BUF = 8.0;
  vector<vector<pair<double, double>>> nWP(NN);
  for (int i = 0; i < NN; i++) {
    string sh = no_fly_zones[i]["shape"].asString();
    nSh[i] = (sh == "circle") ? 0 : 1;
    nTS[i] = no_fly_zones[i]["T_start"].asDouble();
    nTE[i] = no_fly_zones[i]["T_end"].asDouble();
    if (nSh[i] == 0) {
      nCX[i] = no_fly_zones[i]["center"][0].asDouble();
      nCY[i] = no_fly_zones[i]["center"][1].asDouble();
      nR[i] = no_fly_zones[i]["radius"].asDouble();
      for (int j = 0; j < 8; j++) {
        double a = j * 3.14159265358979 / 4;
        nWP[i].push_back(
            {nCX[i] + (nR[i] + BUF) * cos(a), nCY[i] + (nR[i] + BUF) * sin(a)});
      }
    } else {
      nXN[i] = no_fly_zones[i]["corners"][0][0].asDouble();
      nYN[i] = no_fly_zones[i]["corners"][0][1].asDouble();
      nXX[i] = no_fly_zones[i]["corners"][1][0].asDouble();
      nYX[i] = no_fly_zones[i]["corners"][1][1].asDouble();
      nWP[i].push_back({nXN[i] - BUF, nYN[i] - BUF});
      nWP[i].push_back({nXN[i] - BUF, nYX[i] + BUF});
      nWP[i].push_back({nXX[i] + BUF, nYN[i] - BUF});
      nWP[i].push_back({nXX[i] + BUF, nYX[i] + BUF});
    }
  }
  int NCS = (int)charging_stations.size();
  vector<double> csX(NCS), csY(NCS);
  for (int i = 0; i < NCS; i++) {
    csX[i] = charging_stations[i]["x"].asDouble();
    csY[i] = charging_stations[i]["y"].asDouble();
  }

  auto ed = [](double a, double b, double c, double d) -> double {
    return sqrt((a - c) * (a - c) + (b - d) * (b - d));
  };
  auto sB = [&](double ax, double ay, double bx, double by, double t0,
                int i) -> bool {
    double d = ed(ax, ay, bx, by);
    if (d < 1e-9) {
      if (t0 < nTS[i] || t0 > nTE[i])
        return false;
      if (nSh[i] == 0)
        return ed(ax, ay, nCX[i], nCY[i]) <= nR[i];
      return ax >= nXN[i] && ax <= nXX[i] && ay >= nYN[i] && ay <= nYX[i];
    }
    if (nSh[i] == 0) {
      double dx = bx - ax, dy = by - ay, ex2 = ax - nCX[i], ey = ay - nCY[i];
      double a2 = dx * dx + dy * dy, b2 = 2 * (ex2 * dx + ey * dy),
             c2 = ex2 * ex2 + ey * ey - nR[i] * nR[i];
      double disc = b2 * b2 - 4 * a2 * c2;
      if (disc < 0)
        return false;
      double sq = sqrt(disc), s1 = (-b2 - sq) / (2 * a2),
             s2 = (-b2 + sq) / (2 * a2);
      double lo = s1 > 0 ? s1 : 0, hi = s2 < 1 ? s2 : 1;
      double tlo = (nTS[i] - t0) / d, thi = (nTE[i] - t0) / d;
      if (tlo > lo)
        lo = tlo;
      if (thi < hi)
        hi = thi;
      return lo < hi + 1e-9;
    } else {
      double dx = bx - ax, dy = by - ay, txn, txx, tyn, tyx;
      if (dx > -1e-12 && dx < 1e-12) {
        if (ax < nXN[i] || ax > nXX[i])
          return false;
        txn = 0;
        txx = 1;
      } else {
        txn = (nXN[i] - ax) / dx;
        txx = (nXX[i] - ax) / dx;
        if (txn > txx) {
          double t = txn;
          txn = txx;
          txx = t;
        }
      }
      if (dy > -1e-12 && dy < 1e-12) {
        if (ay < nYN[i] || ay > nYX[i])
          return false;
        tyn = 0;
        tyx = 1;
      } else {
        tyn = (nYN[i] - ay) / dy;
        tyx = (nYX[i] - ay) / dy;
        if (tyn > tyx) {
          double t = tyn;
          tyn = tyx;
          tyx = t;
        }
      }
      double lo = txn > tyn ? txn : tyn, hi = txx < tyx ? txx : tyx;
      if (lo < 0)
        lo = 0;
      if (hi > 1)
        hi = 1;
      double tlo = (nTS[i] - t0) / d, thi = (nTE[i] - t0) / d;
      if (tlo > lo)
        lo = tlo;
      if (thi < hi)
        hi = thi;
      return lo < hi + 1e-9;
    }
  };
  auto aB = [&](double ax, double ay, double bx, double by, double t0) -> int {
    for (int i = 0; i < NN; i++)
      if (sB(ax, ay, bx, by, t0, i))
        return i;
    return -1;
  };
  auto fS = [&](double ax, double ay, double bx, double by, double t0) -> PR {
    PR r;
    r.hasWP = false;
    r.wait = 0;
    r.ok = true;
    r.wx = r.wy = 0;
    if (aB(ax, ay, bx, by, t0) < 0)
      return r;
    double maxTE = 0;
    bool perm = false;
    for (int i = 0; i < NN; i++) {
      if (!sB(ax, ay, bx, by, t0, i))
        continue;
      if (nTE[i] > 90000) {
        perm = true;
        break;
      }
      if (nTE[i] > maxTE)
        maxTE = nTE[i];
    }
    if (!perm && maxTE > 0) {
      double wt = maxTE - t0 + 0.5;
      if (wt > 0 && aB(ax, ay, bx, by, t0 + wt) < 0) {
        r.wait = wt;
        return r;
      }
    }
    double bestD = 1e18;
    for (int i = 0; i < NN; i++) {
      if (!sB(ax, ay, bx, by, t0, i))
        continue;
      for (int j = 0; j < (int)nWP[i].size(); j++) {
        double wx = nWP[i][j].first, wy = nWP[i][j].second;
        if (wx < -5 || wx > mapW + 5 || wy < -5 || wy > mapH + 5)
          continue;
        double d1 = ed(ax, ay, wx, wy), d2 = ed(wx, wy, bx, by), td = d1 + d2;
        if (td >= bestD)
          continue;
        if (aB(ax, ay, wx, wy, t0) >= 0)
          continue;
        if (aB(wx, wy, bx, by, t0 + d1) >= 0)
          continue;
        bestD = td;
        r.wx = wx;
        r.wy = wy;
        r.hasWP = true;
      }
    }
    if (r.hasWP)
      return r;
    if (!perm && maxTE > 0) {
      double wt = maxTE - t0 + 0.5;
      if (wt > 0) {
        for (int i = 0; i < NN; i++) {
          for (int j = 0; j < (int)nWP[i].size(); j++) {
            double wx = nWP[i][j].first, wy = nWP[i][j].second,
                   d1 = ed(ax, ay, wx, wy);
            if (aB(ax, ay, wx, wy, t0 + wt) >= 0)
              continue;
            if (aB(wx, wy, bx, by, t0 + wt + d1) >= 0)
              continue;
            r.wait = wt;
            r.wx = wx;
            r.wy = wy;
            r.hasWP = true;
            return r;
          }
        }
      }
    }
    r.ok = false;
    return r;
  };

  // Energy for a trip order
  auto tripE = [&](vector<int> &o) -> double {
    double e = 0, p = 0;
    for (int i : o)
      p += dW[i];
    double x = warehouseX, y = warehouseY;
    for (int i : o) {
      double d = ed(x, y, dX[i], dY[i]);
      e += d * (1 + p);
      p -= dW[i];
      x = dX[i];
      y = dY[i];
    }
    e += ed(x, y, warehouseX, warehouseY);
    return e;
  };

  // 2-opt swap improvement
  auto opt2 = [&](vector<int> &trip) {
    if ((int)trip.size() <= 1)
      return;
    for (int it = 0; it < 3; it++) {
      bool imp = false;
      double be = tripE(trip);
      for (int i = 0; i < (int)trip.size(); i++)
        for (int j = i + 1; j < (int)trip.size(); j++) {
          swap(trip[i], trip[j]);
          double ne = tripE(trip);
          if (ne < be - 0.01) {
            be = ne;
            imp = true;
          } else
            swap(trip[i], trip[j]);
        }
      if (!imp)
        break;
    }
  };

  // Simulate a trip, return (ok, validIndices, pathResults, endTime, endX,
  // endY, endBat)
  struct SimRes {
    bool ok;
    vector<int> vi;
    vector<PR> pr;
    double eT, eX, eY, eB;
  };
  // Can't define inside main for some compilers, but we'll use it via lambda
  // return

  vector<bool> delivered(ND, false);
  vector<bool> failed(ND, false);
  vector<double> drT(NDRONES, 0);

  // Per-drone path storage
  struct Step {
    double x, y, t;
    string action;
    vector<string> dids;
    string did;
  };
  vector<vector<Step>> drPaths(NDRONES);

  // Main loop: round-robin, build trip per drone
  int maxIter = ND * 5, iter = 0;
  while (iter++ < maxIter) {
    // Count undelivered
    int rem = 0;
    for (int i = 0; i < ND; i++)
      if (!delivered[i] && !failed[i])
        rem++;
    if (rem == 0)
      break;

    // Pick earliest-available drone
    int di = -1;
    double bt = 1e18;
    for (int d = 0; d < NDRONES; d++)
      if (drT[d] < bt) {
        bt = drT[d];
        di = d;
      }
    if (di == -1)
      break;
    double cT = drT[di], bat = 500.0;

    // Build trip: nearest neighbor from warehouse, check deadline+payload
    vector<int> trip;
    double tw = 0, px = warehouseX, py = warehouseY, simT = cT;
    // Sort candidates by deadline
    vector<int> cands;
    for (int i = 0; i < ND; i++)
      if (!delivered[i] && !failed[i] && dW[i] <= drMP[di] + 1e-9)
        cands.push_back(i);
    if (cands.empty()) {
      drT[di] = 1e18;
      continue;
    } // no more for this drone

    // Seed: earliest deadline reachable
    sort(cands.begin(), cands.end(),
         [&](int a, int b) { return dDL[a] < dDL[b]; });
    int seed = -1;
    for (int c : cands) {
      double d = ed(warehouseX, warehouseY, dX[c], dY[c]);
      if (cT + d <= dDL[c] + 0.01) {
        seed = c;
        break;
      } else {
        failed[c] = true;
      }
    }
    if (seed < 0) {
      continue;
    }

    trip.push_back(seed);
    tw = dW[seed];
    px = dX[seed];
    py = dY[seed];
    simT = cT + ed(warehouseX, warehouseY, dX[seed], dY[seed]);

    // Add nearest neighbors
    for (int k = 1; k < 25 && k < ND; k++) {
      int best = -1;
      double bd = 1e18;
      for (int i = 0; i < ND; i++) {
        if (delivered[i] || failed[i])
          continue;
        bool inTrip = false;
        for (int t : trip)
          if (t == i) {
            inTrip = true;
            break;
          }
        if (inTrip)
          continue;
        if (tw + dW[i] > drMP[di] + 1e-9)
          continue;
        double d = ed(px, py, dX[i], dY[i]);
        // Rough deadline check
        if (simT + d > dDL[i] + 0.01)
          continue;
        if (d < bd) {
          bd = d;
          best = i;
        }
      }
      if (best < 0)
        break;
      trip.push_back(best);
      tw += dW[best];
      simT += ed(px, py, dX[best], dY[best]);
      px = dX[best];
      py = dY[best];
    }

    // Optimize order
    opt2(trip);

    // Simulate with exact energy + NFZ (Rough check skipping invalid items)
    double sT = cT, sB2 = 500, sP = 0;
    for (int i : trip)
      sP += dW[i];
    double sx = warehouseX, sy = warehouseY;
    bool ok = true;
    vector<int> valid;
    vector<PR> prs;
    for (int k = 0; k < (int)trip.size(); k++) {
      int idx = trip[k];
      PR pr = fS(sx, sy, dX[idx], dY[idx], sT);
      if (!pr.ok) {
        sP -= dW[idx];
        continue;
      }
      double tA = sT + pr.wait, totD = 0;
      if (pr.hasWP) {
        double d1 = ed(sx, sy, pr.wx, pr.wy),
               d2 = ed(pr.wx, pr.wy, dX[idx], dY[idx]);
        totD = d1 + d2;
      } else
        totD = ed(sx, sy, dX[idx], dY[idx]);
      double eU = totD * (1 + sP), tArr = tA + totD;
      if (tArr > dDL[idx] + 0.01) {
        sP -= dW[idx];
        continue;
      }

      double retD = ed(dX[idx], dY[idx], warehouseX, warehouseY);
      PR rPr = fS(dX[idx], dY[idx], warehouseX, warehouseY, tArr);
      if (rPr.ok && rPr.hasWP)
        retD = ed(dX[idx], dY[idx], rPr.wx, rPr.wy) +
               ed(rPr.wx, rPr.wy, warehouseX, warehouseY);
      else if (!rPr.ok)
        retD *= 1.3;

      // Check energy: need to return (possibly via charging station)
      double minRet = retD;
      for (int c = 0; c < NCS; c++) {
        PR cPr = fS(dX[idx], dY[idx], csX[c], csY[c], tArr);
        if (cPr.ok) {
          double dc = ed(dX[idx], dY[idx], csX[c], csY[c]);
          if (cPr.hasWP)
            dc = ed(dX[idx], dY[idx], cPr.wx, cPr.wy) +
                 ed(cPr.wx, cPr.wy, csX[c], csY[c]);
          if (dc < minRet)
            minRet = dc;
        }
      }
      if (sB2 - eU - minRet < -0.01) {
        sP -= dW[idx];
        continue;
      }
      valid.push_back(idx);
      prs.push_back(pr);
      sT = tArr;
      sB2 -= eU;
      sP -= dW[idx];
      sx = dX[idx];
      sy = dY[idx];
    }

    if (valid.empty()) {
      failed[trip.back()] = true;
      continue;
    }

    // Re-simulate with exact payload for valid set
    sT = cT;
    sB2 = 500;
    sP = 0;
    for (int i : valid)
      sP += dW[i];
    sx = warehouseX;
    sy = warehouseY;
    ok = true;
    prs.clear();
    for (int k = 0; k < (int)valid.size(); k++) {
      int idx = valid[k];
      PR pr = fS(sx, sy, dX[idx], dY[idx], sT);
      if (!pr.ok) {
        ok = false;
        break;
      }
      prs.push_back(pr);
      sT += pr.wait;
      if (pr.hasWP) {
        double d1 = ed(sx, sy, pr.wx, pr.wy),
               d2 = ed(pr.wx, pr.wy, dX[idx], dY[idx]);
        sB2 -= d1 * (1 + sP);
        sT += d1;
        sB2 -= d2 * (1 + sP);
        sT += d2;
      } else {
        double d = ed(sx, sy, dX[idx], dY[idx]);
        sB2 -= d * (1 + sP);
        sT += d;
      }
      if (sB2 < -0.01 || sT > dDL[idx] + 0.01) {
        ok = false;
        break;
      }
      sP -= dW[idx];
      sx = dX[idx];
      sy = dY[idx];
    }

    // Check return energy - compute actual return distance including NFZ
    // waypoints
    double retDW = ed(sx, sy, warehouseX, warehouseY);
    bool useCS = false;
    int bestCS = -1;
    if (ok) {
      PR retPR = fS(sx, sy, warehouseX, warehouseY, sT);
      if (retPR.ok && retPR.hasWP)
        retDW = ed(sx, sy, retPR.wx, retPR.wy) +
                ed(retPR.wx, retPR.wy, warehouseX, warehouseY);
      else if (!retPR.ok)
        retDW *= 1.3;

      if (sB2 - retDW < -0.01) {
        // Try charging station
        for (int c = 0; c < NCS; c++) {
          PR csPR = fS(sx, sy, csX[c], csY[c], sT);
          if (csPR.ok) {
            double dc = ed(sx, sy, csX[c], csY[c]);
            if (csPR.hasWP)
              dc = ed(sx, sy, csPR.wx, csPR.wy) +
                   ed(csPR.wx, csPR.wy, csX[c], csY[c]);
            if (sB2 - dc >= -0.01 &&
                (bestCS < 0 || dc < ed(sx, sy, csX[bestCS], csY[bestCS]))) {
              bestCS = c;
              useCS = true;
            }
          }
        }
        if (!useCS)
          ok = false;
      }
    }
    if (ok && !useCS && sB2 - retDW < -0.01)
      ok = false;

    // Trim if needed
    while (!ok && (int)valid.size() > 1) {
      valid.pop_back();
      prs.clear();
      sT = cT;
      sB2 = 500;
      sP = 0;
      for (int i : valid)
        sP += dW[i];
      sx = warehouseX;
      sy = warehouseY;
      ok = true;
      for (int k = 0; k < (int)valid.size(); k++) {
        int idx = valid[k];
        PR pr = fS(sx, sy, dX[idx], dY[idx], sT);
        if (!pr.ok) {
          ok = false;
          break;
        }
        prs.push_back(pr);
        sT += pr.wait;
        if (pr.hasWP) {
          double d1 = ed(sx, sy, pr.wx, pr.wy),
                 d2 = ed(pr.wx, pr.wy, dX[idx], dY[idx]);
          sB2 -= d1 * (1 + sP);
          sT += d1;
          sB2 -= d2 * (1 + sP);
          sT += d2;
        } else {
          double d = ed(sx, sy, dX[idx], dY[idx]);
          sB2 -= d * (1 + sP);
          sT += d;
        }
        if (sB2 < -0.01 || sT > dDL[idx] + 0.01) {
          ok = false;
          break;
        }
        sP -= dW[idx];
        sx = dX[idx];
        sy = dY[idx];
      }
      if (ok) {
        retDW = ed(sx, sy, warehouseX, warehouseY);
        PR retPR2 = fS(sx, sy, warehouseX, warehouseY, sT);
        if (retPR2.ok && retPR2.hasWP)
          retDW = ed(sx, sy, retPR2.wx, retPR2.wy) +
                  ed(retPR2.wx, retPR2.wy, warehouseX, warehouseY);
        else if (!retPR2.ok)
          retDW *= 1.3;

        useCS = false;
        bestCS = -1;
        if (sB2 - retDW < -0.01) {
          for (int c = 0; c < NCS; c++) {
            PR csPR = fS(sx, sy, csX[c], csY[c], sT);
            if (csPR.ok) {
              double dc = ed(sx, sy, csX[c], csY[c]);
              if (csPR.hasWP)
                dc = ed(sx, sy, csPR.wx, csPR.wy) +
                     ed(csPR.wx, csPR.wy, csX[c], csY[c]);
              if (sB2 - dc >= -0.01 &&
                  (bestCS < 0 || dc < ed(sx, sy, csX[bestCS], csY[bestCS]))) {
                bestCS = c;
                useCS = true;
              }
            }
          }
          if (!useCS)
            ok = false;
        }
      }
    }

    if (!ok || valid.empty()) {
      failed[valid.empty() ? seed : valid[0]] = true;
      continue;
    }

    // Commit: mark delivered
    for (int i : valid)
      delivered[i] = true;

    // Generate path steps
    vector<Step> &path = drPaths[di];
    double pT = cT, pB = 500, pP = 0, pX = warehouseX, pY = warehouseY;
    for (int i : valid)
      pP += dW[i];
    // PICKUP
    Step pu;
    pu.x = warehouseX;
    pu.y = warehouseY;
    pu.t = pT;
    pu.action = "PICKUP";
    for (int i : valid)
      pu.dids.push_back(dId[i]);
    path.push_back(pu);

    for (int k = 0; k < (int)valid.size(); k++) {
      int idx = valid[k];
      PR &pr = prs[k];
      if (pr.wait > 0.001) {
        pT += pr.wait;
        Step w;
        w.x = pX;
        w.y = pY;
        w.t = pT;
        w.action = "WAIT";
        path.push_back(w);
      }
      if (pr.hasWP) {
        double d1 = ed(pX, pY, pr.wx, pr.wy);
        pB -= d1 * (1 + pP);
        pT += d1;
        Step w;
        w.x = pr.wx;
        w.y = pr.wy;
        w.t = pT;
        w.action = "WAYPOINT";
        path.push_back(w);
        pX = pr.wx;
        pY = pr.wy;
      }
      double d = ed(pX, pY, dX[idx], dY[idx]);
      pB -= d * (1 + pP);
      pT += d;
      pP -= dW[idx];
      pX = dX[idx];
      pY = dY[idx];
      Step dp;
      dp.x = dX[idx];
      dp.y = dY[idx];
      dp.t = pT;
      dp.action = "DELIVER";
      dp.did = dId[idx];
      path.push_back(dp);
    }

    // Return (possibly via charging station)
    PR rp = fS(pX, pY, warehouseX, warehouseY, pT);
    if (useCS && bestCS >= 0) {
      // Go to charging station first
      PR cp = fS(pX, pY, csX[bestCS], csY[bestCS], pT);
      if (cp.ok) {
        if (cp.wait > 0.001) {
          pT += cp.wait;
          Step w;
          w.x = pX;
          w.y = pY;
          w.t = pT;
          w.action = "WAIT";
          path.push_back(w);
        }
        if (cp.hasWP) {
          double d1 = ed(pX, pY, cp.wx, cp.wy);
          pB -= d1;
          pT += d1;
          Step w;
          w.x = cp.wx;
          w.y = cp.wy;
          w.t = pT;
          w.action = "WAYPOINT";
          path.push_back(w);
          pX = cp.wx;
          pY = cp.wy;
        }
        double dc = ed(pX, pY, csX[bestCS], csY[bestCS]);
        pB -= dc;
        pT += dc;
        pX = csX[bestCS];
        pY = csY[bestCS];
        Step ch;
        ch.x = pX;
        ch.y = pY;
        ch.t = pT;
        ch.action = "CHARGE";
        path.push_back(ch);
        double dw = ed(pX, pY, warehouseX, warehouseY) * 1.3;
        if (dw > 500)
          dw = 500;
        double need = dw - pB;
        if (need > 0) {
          double ct2 = ceil(need / 2.0);
          pB += ct2 * 2;
          if (pB > 500)
            pB = 500;
          pT += ct2;
        }
        Step cc;
        cc.x = pX;
        cc.y = pY;
        cc.t = pT;
        cc.action = "CHARGE_COMPLETE";
        path.push_back(cc);
      }
      rp = fS(pX, pY, warehouseX, warehouseY, pT);
    }
    if (rp.ok && rp.wait > 0.001) {
      pT += rp.wait;
      Step w;
      w.x = pX;
      w.y = pY;
      w.t = pT;
      w.action = "WAIT";
      path.push_back(w);
    }
    if (rp.ok && rp.hasWP) {
      double d1 = ed(pX, pY, rp.wx, rp.wy);
      pB -= d1;
      pT += d1;
      Step w;
      w.x = rp.wx;
      w.y = rp.wy;
      w.t = pT;
      w.action = "WAYPOINT";
      path.push_back(w);
      pX = rp.wx;
      pY = rp.wy;
    }
    double rd = ed(pX, pY, warehouseX, warehouseY);
    pT += rd;
    Step rt;
    rt.x = warehouseX;
    rt.y = warehouseY;
    rt.t = pT;
    rt.action = "RETURN";
    path.push_back(rt);
    drT[di] = pT;
  }

  // Build JSON output
  Json::Value fm(Json::arrayValue);
  for (int di = 0; di < NDRONES; di++) {
    if (drPaths[di].empty())
      continue;
    Json::Value de;
    de["drone_id"] = drId[di];
    Json::Value fp(Json::arrayValue);
    for (auto &s : drPaths[di]) {
      Json::Value st;
      st["x"] = s.x;
      st["y"] = s.y;
      st["t"] = s.t;
      st["action"] = s.action;
      if (!s.dids.empty()) {
        Json::Value ids(Json::arrayValue);
        for (auto &id : s.dids)
          ids.append(id);
        st["delivery_ids"] = ids;
      }
      if (!s.did.empty())
        st["delivery_id"] = s.did;
      fp.append(st);
    }
    de["path"] = fp;
    fm.append(de);
  }
  // End of BODY

  // Start of TAIL
  Json::Value output;
  output["flight_manifest"] = fm;
  Json::StreamWriterBuilder wb;
  wb["indentation"] = "";
  cout << Json::writeString(wb, output) << endl;
  return 0;
}
// End of TAIL
