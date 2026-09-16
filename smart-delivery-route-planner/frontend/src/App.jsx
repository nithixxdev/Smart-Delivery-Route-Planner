import React, { useState, useMemo } from "react";

const initialStops = [
  { id: "A", name: "Anna Nagar", x: 22, y: 27, demand: 5, priority: 3, readyHour: 9, dueHour: 12, serviceMinutes: 8 },
  { id: "B", name: "T. Nagar", x: 71, y: 25, demand: 4, priority: 2, readyHour: 9, dueHour: 15, serviceMinutes: 6 },
  { id: "C", name: "Adyar", x: 78, y: 68, demand: 7, priority: 5, readyHour: 10, dueHour: 13, serviceMinutes: 10 },
  { id: "D", name: "Velachery", x: 54, y: 80, demand: 3, priority: 1, readyHour: 11, dueHour: 17, serviceMinutes: 7 }
];

function hourLabel(h) {
  const hour = Math.floor(h);
  const mins = Math.round((h - hour) * 60);
  const suffix = hour >= 12 ? "PM" : "AM";
  const display = hour % 12 || 12;
  return `${display}:${String(mins).padStart(2, "0")} ${suffix}`;
}

function Map({ depot, stops, route }) {
  const points = useMemo(() => {
    const byId = new globalThis.Map(stops.map(s => [s.id, s]));
    return route?.length
      ? route.map(r => byId.get(r.id)).filter(Boolean)
      : [];
  }, [route, stops]);

  return (
    <div className="map">
      <div className="grid-lines" />
      {points.length > 0 && (
        <svg className="route-svg" viewBox="0 0 100 100" preserveAspectRatio="none">
          <line x1={depot.x} y1={depot.y} x2={points[0].x} y2={points[0].y} />
          {points.map((p, i) => {
            const next = points[i + 1];
            return next ? <line key={`${p.id}-${next.id}`} x1={p.x} y1={p.y} x2={next.x} y2={next.y} /> : null;
          })}
          {points.length && <line x1={points.at(-1).x} y1={points.at(-1).y} x2={depot.x} y2={depot.y} />}
        </svg>
      )}
      <div className="map-label">Route workspace</div>
      <div className="marker depot" style={{ left: `${depot.x}%`, top: `${depot.y}%` }}>
        <span>◆</span><small>Depot</small>
      </div>
      {stops.map((s) => {
        const sequence = route?.findIndex(r => r.id === s.id);
        return (
          <div key={s.id} className={`marker stop ${sequence >= 0 ? "served" : ""}`}
            style={{ left: `${s.x}%`, top: `${s.y}%` }}>
            <span>{sequence >= 0 ? sequence + 1 : s.id}</span>
            <small>{s.name}</small>
          </div>
        );
      })}
    </div>
  );
}

export default function App() {
  const [stops, setStops] = useState(initialStops);
  const [capacity, setCapacity] = useState(30);
  const [startHour, setStartHour] = useState(9);
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  const depot = { id: "DEPOT", name: "Central Depot", x: 50, y: 50 };

  function updateStop(id, key, value) {
    setStops(items => items.map(s => s.id === id ? {
      ...s,
      [key]: ["demand", "priority", "readyHour", "dueHour", "serviceMinutes", "x", "y"].includes(key)
        ? Number(value) : value
    } : s));
  }

  function addStop() {
    const id = String.fromCharCode(65 + stops.length);
    setStops([...stops, {
      id, name: `Customer ${id}`, x: 30 + ((stops.length * 17) % 50),
      y: 25 + ((stops.length * 23) % 55), demand: 2, priority: 2,
      readyHour: 9, dueHour: 17, serviceMinutes: 5
    }]);
  }

  function removeStop(id) {
    setStops(stops.filter(s => s.id !== id));
    setResult(null);
  }

  async function optimize() {
    setLoading(true);
    setError("");
    try {
      const response = await fetch("http://localhost:18080/api/optimize", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ depot, stops, vehicleCapacity: capacity, startHour })
      });
      if (!response.ok) throw new Error(await response.text());
      setResult(await response.json());
    } catch (e) {
      setError("Could not reach the C++ backend. Start the backend on port 18080 and try again.");
    } finally {
      setLoading(false);
    }
  }

  const totalDemand = stops.reduce((a, s) => a + s.demand, 0);

  return (
    <main>
      <header className="topbar">
        <div className="brand"><span className="brand-mark">S</span><div><strong>SmartRoute</strong><small>Delivery intelligence</small></div></div>
        <div className="status"><span className="dot" /> C++ optimizer ready</div>
      </header>

      <section className="hero">
        <div>
          <div className="eyebrow">ROUTE PLANNING WORKSPACE</div>
          <h1>Plan smarter.<br /><em>Deliver on time.</em></h1>
          <p>Build an explainable delivery route using distance, priority, time windows and vehicle capacity.</p>
        </div>
        <div className="hero-card">
          <span>Today's planning load</span>
          <strong>{stops.length} <small>stops</small></strong>
          <div className="mini-bar"><i style={{ width: `${Math.min(100, totalDemand / capacity * 100)}%` }} /></div>
          <small>{totalDemand} / {capacity} capacity units</small>
        </div>
      </section>

      <section className="workspace">
        <aside className="panel controls">
          <div className="panel-title"><div><span className="step">01</span><h2>Route inputs</h2></div><span className="muted">Configure</span></div>

          <label>Vehicle capacity <b>{capacity}</b></label>
          <input type="range" min="5" max="60" value={capacity} onChange={e => setCapacity(Number(e.target.value))} />

          <label>Departure time <b>{hourLabel(startHour)}</b></label>
          <input type="range" min="6" max="14" step="0.5" value={startHour} onChange={e => setStartHour(Number(e.target.value))} />

          <div className="stops-head"><span>DELIVERY STOPS</span><button onClick={addStop}>+ Add stop</button></div>

          <div className="stop-list">
            {stops.map((s) => (
              <article className="stop-card" key={s.id}>
                <div className="stop-top"><span className="stop-id">{s.id}</span><input value={s.name} onChange={e => updateStop(s.id, "name", e.target.value)} /><button onClick={() => removeStop(s.id)}>×</button></div>
                <div className="fields">
                  <div><small>Demand</small><input type="number" min="0" value={s.demand} onChange={e => updateStop(s.id, "demand", e.target.value)} /></div>
                  <div><small>Priority</small><input type="number" min="1" max="5" value={s.priority} onChange={e => updateStop(s.id, "priority", e.target.value)} /></div>
                  <div><small>Ready</small><input type="number" min="0" max="23" value={s.readyHour} onChange={e => updateStop(s.id, "readyHour", e.target.value)} /></div>
                  <div><small>Due</small><input type="number" min="1" max="23" value={s.dueHour} onChange={e => updateStop(s.id, "dueHour", e.target.value)} /></div>
                </div>
              </article>
            ))}
          </div>

          <button className="optimize" onClick={optimize} disabled={loading || stops.length === 0}>
            {loading ? "Optimizing…" : "Generate smart route →"}
          </button>
          {error && <div className="error">{error}</div>}
        </aside>

        <section className="panel map-panel">
          <div className="panel-title"><div><span className="step">02</span><h2>Route view</h2></div><span className="muted">{result ? "Optimized" : "Preview"}</span></div>
          <Map depot={depot} stops={stops} route={result?.route} />

          <div className="metrics">
            <div><small>DISTANCE</small><strong>{result ? result.totalDistance.toFixed(1) : "—"} <i>units</i></strong></div>
            <div><small>EST. DURATION</small><strong>{result ? Math.round(result.totalDurationMinutes) : "—"} <i>min</i></strong></div>
            <div><small>STOPS SERVED</small><strong>{result ? result.stopsServed : "—"} <i>/ {stops.length}</i></strong></div>
            <div><small>CAPACITY</small><strong>{result ? result.capacityUsed : totalDemand} <i>/ {capacity}</i></strong></div>
          </div>
        </section>
      </section>

      <section className="panel results">
        <div className="panel-title"><div><span className="step">03</span><h2>Delivery sequence</h2></div><span className={`badge ${result?.feasible === false ? "bad" : ""}`}>{result ? (result.feasible ? "Route ready" : "Needs attention") : "Waiting for route"}</span></div>

        {!result ? (
          <div className="empty"><div className="empty-icon">↗</div><h3>Your route will appear here</h3><p>Configure the stops and generate a route to see the delivery sequence and arrival estimates.</p></div>
        ) : (
          <>
            <div className="sequence">
              {result.route.map((r) => (
                <div className="sequence-row" key={r.id}>
                  <span className="sequence-number">{r.sequence}</span>
                  <div className="sequence-main"><strong>{r.name}</strong><small>Priority {r.priority} · {r.demand} units</small></div>
                  <div><small>ARRIVAL</small><strong>{hourLabel(r.arrivalHour)}</strong></div>
                  <div><small>WINDOW</small><strong>{hourLabel(r.readyHour)}–{hourLabel(r.dueHour)}</strong></div>
                  <div><small>LEG</small><strong>{r.distanceFromPrevious.toFixed(1)} units</strong></div>
                </div>
              ))}
            </div>
            {result.warnings.length > 0 && <div className="warnings"><strong>Planning notes</strong>{result.warnings.map((w, i) => <div key={i}>• {w}</div>)}</div>}
          </>
        )}
      </section>

      <footer>SmartRoute · C++20 route optimization engine · Built as an engineering portfolio project</footer>
    </main>
  );
}
