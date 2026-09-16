#pragma once

#include <string>
#include <vector>

namespace smartroute {

struct Location {
    std::string id;
    std::string name;
    double x{};
    double y{};
};

struct DeliveryStop : Location {
    int demand{};
    int priority{1};       // 1 = normal, 5 = highest
    double readyHour{9.0};
    double dueHour{18.0};
    int serviceMinutes{5};
};

struct RoutePoint {
    DeliveryStop stop;
    double arrivalHour{};
    double departureHour{};
    double distanceFromPrevious{};
};

struct OptimizationRequest {
    Location depot;
    std::vector<DeliveryStop> stops;
    int vehicleCapacity{30};
    double startHour{9.0};
};

struct OptimizationResult {
    std::vector<RoutePoint> route;
    double totalDistance{};
    double totalDurationMinutes{};
    int capacityUsed{};
    bool feasible{true};
    std::vector<std::string> warnings;
};

OptimizationResult optimize(const OptimizationRequest& request);

} // namespace smartroute
