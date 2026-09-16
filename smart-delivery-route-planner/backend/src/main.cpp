#include "route_optimizer.hpp"

#include "crow.h"

#include <algorithm>
#include <cstdlib>
#include <string>

using smartroute::DeliveryStop;
using smartroute::Location;
using smartroute::OptimizationRequest;

namespace {

double number(const crow::json::rvalue& obj, const char* key, double fallback) {
    if (!obj.has(key)) return fallback;
    return obj[key].d();
}

int integer(const crow::json::rvalue& obj, const char* key, int fallback) {
    if (!obj.has(key)) return fallback;
    return obj[key].i();
}

std::string text(const crow::json::rvalue& obj, const char* key,
                 const std::string& fallback = "") {
    if (!obj.has(key)) return fallback;
    return obj[key].s();
}

Location parseLocation(const crow::json::rvalue& obj) {
    Location location;
    location.id = text(obj, "id");
    location.name = text(obj, "name", location.id);
    location.x = number(obj, "x", 0);
    location.y = number(obj, "y", 0);
    return location;
}

DeliveryStop parseStop(const crow::json::rvalue& obj) {
    DeliveryStop stop;
    const Location base = parseLocation(obj);
    stop.id = base.id;
    stop.name = base.name;
    stop.x = base.x;
    stop.y = base.y;
    stop.demand = integer(obj, "demand", 1);
    stop.priority = std::clamp(integer(obj, "priority", 1), 1, 5);
    stop.readyHour = number(obj, "readyHour", 9);
    stop.dueHour = number(obj, "dueHour", 18);
    stop.serviceMinutes = std::max(0, integer(obj, "serviceMinutes", 5));
    return stop;
}

crow::json::wvalue resultJson(const smartroute::OptimizationResult& result) {
    crow::json::wvalue out;
    out["feasible"] = result.feasible;
    out["totalDistance"] = result.totalDistance;
    out["totalDurationMinutes"] = result.totalDurationMinutes;
    out["capacityUsed"] = result.capacityUsed;
    out["stopsServed"] = static_cast<int>(result.route.size());

    crow::json::wvalue route = crow::json::wvalue::list();
    int index = 0;
    for (const auto& p : result.route) {
        crow::json::wvalue item;
        item["sequence"] = ++index;
        item["id"] = p.stop.id;
        item["name"] = p.stop.name;
        item["x"] = p.stop.x;
        item["y"] = p.stop.y;
        item["demand"] = p.stop.demand;
        item["priority"] = p.stop.priority;
        item["readyHour"] = p.stop.readyHour;
        item["dueHour"] = p.stop.dueHour;
        item["arrivalHour"] = p.arrivalHour;
        item["departureHour"] = p.departureHour;
        item["distanceFromPrevious"] = p.distanceFromPrevious;
        route[index - 1] = std::move(item);
    }
    out["route"] = std::move(route);

    crow::json::wvalue warnings = crow::json::wvalue::list();
    for (std::size_t i = 0; i < result.warnings.size(); ++i)
        warnings[i] = result.warnings[i];
    out["warnings"] = std::move(warnings);

    return out;
}

} // namespace

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/health")
    ([] {
        crow::json::wvalue out;
        out["status"] = "ok";
        out["service"] = "smartroute";
        return out;
    });

    CROW_ROUTE(app, "/api/optimize")
        .methods(crow::HTTPMethod::POST)
    ([](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                return crow::response(400, "Invalid JSON.");
            }

            if (!body.has("depot") || !body.has("stops")) {
                return crow::response(400,
                    "Request requires 'depot' and 'stops'.");
            }

            OptimizationRequest request;
            request.depot = parseLocation(body["depot"]);
            request.vehicleCapacity = integer(body, "vehicleCapacity", 30);
            request.startHour = number(body, "startHour", 9);

            for (const auto& item : body["stops"])
                request.stops.push_back(parseStop(item));

            auto result = smartroute::optimize(request);

            crow::response response(resultJson(result));
            response.set_header("Access-Control-Allow-Origin", "*");
            response.set_header("Content-Type", "application/json");
            return response;
        } catch (const std::exception& ex) {
            return crow::response(400, std::string("Request error: ") + ex.what());
        }
    });

    CROW_ROUTE(app, "/api/optimize")
        .methods(crow::HTTPMethod::OPTIONS)
    ([] {
        crow::response response;
        response.code = 204;
        response.set_header("Access-Control-Allow-Origin", "*");
        response.set_header("Access-Control-Allow-Headers", "Content-Type");
        response.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        return response;
    });

    CROW_ROUTE(app, "/")
    ([] {
        return "SmartRoute API";
    });

    app.port(18080).multithreaded().run();
}
