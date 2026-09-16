#include "route_optimizer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

namespace smartroute {

namespace {
double distance(const Location& a, const Location& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

double travelMinutes(double d) {
    // Demo map: 1 distance unit = 1.6 minutes of travel.
    return d * 1.6;
}

double urgency(const DeliveryStop& s, double currentHour) {
    const double remaining = std::max(0.05, s.dueHour - currentHour);
    const double windowWidth = std::max(0.25, s.dueHour - s.readyHour);

    // Higher priority and tighter windows increase score.
    return (s.priority * 2.0) + (1.0 / remaining) * 8.0
           + (1.0 / windowWidth) * 1.5;
}
}

OptimizationResult optimize(const OptimizationRequest& request) {
    OptimizationResult result;

    if (request.vehicleCapacity <= 0) {
        result.feasible = false;
        result.warnings.push_back("Vehicle capacity must be greater than zero.");
        return result;
    }

    std::vector<DeliveryStop> remaining;
    for (const auto& stop : request.stops) {
        if (stop.demand < 0) {
            result.warnings.push_back(stop.name + ": negative demand ignored.");
            continue;
        }
        if (stop.demand > request.vehicleCapacity) {
            result.feasible = false;
            result.warnings.push_back(
                stop.name + ": demand exceeds vehicle capacity.");
        } else {
            remaining.push_back(stop);
        }
    }

    Location current = request.depot;
    double currentHour = request.startHour;
    int capacityUsed = 0;

    while (!remaining.empty()) {
        int bestIndex = -1;
        double bestScore = std::numeric_limits<double>::infinity();
        double bestArrival = 0.0;

        for (int i = 0; i < static_cast<int>(remaining.size()); ++i) {
            const auto& candidate = remaining[i];

            if (capacityUsed + candidate.demand > request.vehicleCapacity)
                continue;

            const double d = distance(current, candidate);
            const double arrival = currentHour + travelMinutes(d) / 60.0;
            const double actualArrival = std::max(arrival, candidate.readyHour);

            // Penalize late arrivals heavily; reward urgency and priority.
            const double lateHours = std::max(0.0, actualArrival - candidate.dueHour);
            const double score =
                d * 1.0
                - urgency(candidate, currentHour) * 2.5
                + lateHours * 500.0;

            if (score < bestScore) {
                bestScore = score;
                bestIndex = i;
                bestArrival = actualArrival;
            }
        }

        if (bestIndex == -1) {
            result.feasible = false;
            result.warnings.push_back(
                "Remaining deliveries cannot fit within the vehicle capacity.");
            break;
        }

        DeliveryStop selected = remaining[bestIndex];
        const double d = distance(current, selected);
        const double arrival = bestArrival;
        const double departure =
            arrival + static_cast<double>(selected.serviceMinutes) / 60.0;

        if (arrival > selected.dueHour) {
            std::ostringstream oss;
            oss << selected.name << ": estimated arrival is after its "
                << "delivery window.";
            result.warnings.push_back(oss.str());
        }

        RoutePoint point;
        point.stop = selected;
        point.arrivalHour = arrival;
        point.departureHour = departure;
        point.distanceFromPrevious = d;

        result.route.push_back(point);
        result.totalDistance += d;
        capacityUsed += selected.demand;
        currentHour = departure;
        current = selected;

        remaining.erase(remaining.begin() + bestIndex);
    }

    // Return-to-depot leg.
    if (!result.route.empty()) {
        const double back = distance(current, request.depot);
        result.totalDistance += back;
        currentHour += travelMinutes(back) / 60.0;
    }

    result.capacityUsed = capacityUsed;
    result.totalDurationMinutes =
        std::max(0.0, (currentHour - request.startHour) * 60.0);

    if (result.route.empty() && !request.stops.empty()) {
        result.feasible = false;
        result.warnings.push_back("No delivery could be scheduled.");
    }

    return result;
}

} // namespace smartroute
