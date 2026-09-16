# SmartRoute — Smart Delivery Route Planner

A resume-ready full-stack delivery route planner built with **C++20 + Crow** on the backend and **React + Vite** on the frontend.

## What it solves

Given a depot and multiple delivery stops, SmartRoute finds a practical route while considering:

- travel distance
- delivery priority
- delivery time windows
- vehicle capacity
- stop service time

The optimizer uses a nearest-feasible-stop heuristic with a priority/time-window score, then reports route distance, estimated duration, capacity usage, and warnings.

> This is an application-level optimization project, not a system/CPU monitoring project.

## Stack

- C++20
- Crow HTTP framework
- CMake
- React
- Vite
- Plain CSS
- JSON over REST

Crow provides C++ HTTP/JSON routing; the current project structure follows its CMake integration model. React/Vite provide the browser UI and development/build workflow.

## Prerequisites

- C++ compiler with C++20 support
- CMake 3.20+
- Git
- Node.js 20.19+ or 22.12+
- npm

## Run backend

From `backend/`:

```bash
cmake -S . -B build
cmake --build build --config Release
```

Then run:

Windows:
```powershell
.\build\Release\smartroute.exe
```

Linux/macOS:
```bash
./build/smartroute
```

The API listens on `http://localhost:18080`.

### Crow dependency

The CMake file fetches Crow automatically through CMake's FetchContent, so you do not need to manually copy Crow headers.

If your network blocks dependency downloads, install Crow separately and adjust `CMakeLists.txt`.

## Run frontend

From `frontend/`:

```bash
npm install
npm run dev
```

Open the URL printed by Vite, normally `http://localhost:5173`.

## API

### GET `/api/health`

Returns:
```json
{"status":"ok","service":"smartroute"}
```

### POST `/api/optimize`

Example request:

```json
{
  "vehicleCapacity": 30,
  "startHour": 9,
  "depot": {"id":"DEPOT","name":"Central Depot","x":50,"y":50},
  "stops": [
    {"id":"A","name":"Customer A","x":20,"y":25,"demand":6,"priority":3,"readyHour":9,"dueHour":12,"serviceMinutes":8},
    {"id":"B","name":"Customer B","x":80,"y":20,"demand":4,"priority":1,"readyHour":10,"dueHour":16,"serviceMinutes":5}
  ]
}
```

## Project structure

```text
smart-delivery-route-planner/
├── backend/
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── route_optimizer.hpp
│   └── src/
│       ├── main.cpp
│       └── route_optimizer.cpp
├── frontend/
│   ├── package.json
│   ├── index.html
│   └── src/
│       ├── main.jsx
│       ├── App.jsx
│       └── styles.css
└── README.md
```

## Resume description

**SmartRoute — Smart Delivery Route Planner | C++20, Crow, React, CMake**

- Built a full-stack delivery route optimization application that generates practical multi-stop routes using distance, delivery priority, time windows, service time, and vehicle-capacity constraints.
- Implemented the route optimizer in C++20 and exposed it through a REST API using Crow, with a React/Vite interface for interactive route planning and visualization.
- Added route diagnostics including estimated arrival times, capacity utilization, distance, duration, missed-window warnings, and optimization rationale.

## Interview explanation

The easiest way to explain the project is:

> “I built a delivery planner where the user enters delivery locations, priorities, time windows, and package quantities. The C++ backend scores feasible next stops based on distance and urgency, builds a route, and returns the estimated schedule. The React frontend visualizes the route and explains the result.”

Important: this implementation intentionally uses an explainable heuristic rather than claiming to solve the globally optimal vehicle-routing problem.
