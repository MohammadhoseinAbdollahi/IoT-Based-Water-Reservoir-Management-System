import React, { useState, useEffect, useRef, useMemo } from "react";
import mqtt from "mqtt";
import GaugeChart from "react-gauge-chart";
import { Line } from "react-chartjs-2";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from "chart.js";

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend);

const App = () => {
  const [temperature, setTemperature] = useState(0);
  const [humidity, setHumidity] = useState(0);
  const [waterLevel, setWaterLevel] = useState(0);
  const [turbidity, setTurbidity] = useState(0);
  const [floodProbability, setFloodProbability] = useState(0);
  // eslint-disable-next-line no-unused-vars
  const [floodWarning, setFloodWarning] = useState(false);
  const [isDamOpen, setIsDamOpen] = useState(false);
  const [inflowData, setInflowData] = useState([]);
  const [outflowData, setOutflowData] = useState([]);
  const [timestamps, setTimestamps] = useState([]);
  const [loading, setLoading] = useState(false);

  const brokerUrl = "wss://9fbc1787c51542aca42e34cc0bb2fbcd.s1.eu.hivemq.cloud:8884/mqtt";

  const options = useMemo(() => ({
    username: "KarkheDam",
    password: "Password1",
    protocol: "wss",
    clientId: `karkhe-dam-dashboard-${Math.random().toString(16).slice(2)}`,
    reconnectPeriod: 3000,
  }), []);

  const clientRef = useRef(null);

  const sendNotification = (title, body) => {
    if (Notification.permission === "granted") {
      new Notification(title, { body, icon: "/warning-icon.png" });
    } else if (Notification.permission !== "denied") {
      Notification.requestPermission().then((permission) => {
        if (permission === "granted") {
          new Notification(title, { body, icon: "/warning-icon.png" });
        }
      });
    }
  };

  useEffect(() => {
    if (!clientRef.current || clientRef.current.disconnected) {
      clientRef.current = mqtt.connect(brokerUrl, options);

      clientRef.current.on("connect", () => {
        console.log("Connected to MQTT broker");

        if (clientRef.current.connected) {
          clientRef.current.subscribe("KarkheDam_Simulation/#", (err) => {
            if (err) {
              console.error("Subscription error:", err);
            } else {
              console.log("Subscribed to KarkheDam_Simulation/#");
            }
          });
        }
      });

      clientRef.current.on("message", (topic, message) => {
        const value = parseFloat(message.toString());
        const now = new Date().toLocaleTimeString();
        console.log(`Received message on topic ${topic}: ${value}`);

        switch (topic) {
          case "KarkheDam_Simulation/temperature":
            setTemperature(value);
            break;
          case "KarkheDam_Simulation/humidity":
            setHumidity(value);
            break;
          case "KarkheDam_Simulation/waterLevel":
            setWaterLevel(value);
            break;
          case "KarkheDam_Simulation/turbidity":
            setTurbidity(value);
            break;
          case "KarkheDam_Simulation/floodProbability":
            setFloodProbability(value);
            break;
          case "KarkheDam_Simulation/floodWarning":
            setFloodWarning(value === 1);
            if (value === 1 && !isDamOpen) {
              sendNotification("Flood Warning!", "Flood warning! Please open the dam!");
            }
            break;
          case "KarkheDam_Simulation/inflowRate":
            setInflowData((prev) => [...prev.slice(-9), value]);
            setTimestamps((prev) => [...prev.slice(-9), now]);
            break;
          case "KarkheDam_Simulation/outflowRate":
            setOutflowData((prev) => [...prev.slice(-9), value]);
            if (value > 300) {
              setIsDamOpen(true);
              sendNotification("Dam Opened", "The dam is open, releasing water.");
            }
            break;
          default:
            console.warn(`Unhandled topic: ${topic}`);
            break;
        }
        setLoading(false);
      });

      clientRef.current.on("error", (error) => {
        console.error("MQTT connection error:", error);
      });

      clientRef.current.on("disconnect", () => {
        console.warn("MQTT client disconnected.");
        setTimeout(() => {
          if (clientRef.current && clientRef.current.disconnected) {
            console.log("Reconnecting to MQTT broker...");
            clientRef.current.reconnect();
          }
        }, 5000);
      });
    }

    return () => {
      if (clientRef.current) {
        clientRef.current.end();
        clientRef.current = null;
      }
    };
  }, [brokerUrl, options, isDamOpen]);

  const openDam = () => {
    if (clientRef.current && clientRef.current.connected) {
      clientRef.current.publish("KarkheDam_Simulation/openDam", "true", (err) => {
        if (err) {
          console.error("Failed to send openDam command:", err);
        } else {
          console.log("Dam opening command sent");
          setIsDamOpen(true);
        }
      });
    } else {
      console.warn("MQTT client is not connected.");
    }
  };

  const lineChartData = {
    labels: timestamps,
    datasets: [
      {
        label: "Inflow Rate",
        data: inflowData,
        borderColor: "rgba(75, 192, 192, 1)",
        backgroundColor: "rgba(75, 192, 192, 0.2)",
        tension: 0.4,
      },
      {
        label: "Outflow Rate",
        data: outflowData,
        borderColor: "rgba(255, 99, 132, 1)",
        backgroundColor: "rgba(255, 99, 132, 0.2)",
        tension: 0.4,
      },
    ],
  };

  if (loading) {
    return <div>Loading... Connecting to simulation...</div>;
  }

  return (
      <div style={{ fontFamily: "Arial, sans-serif", padding: "20px" }}>
        <h1>Karkhe Dam Dashboard</h1>
        <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center" }}>
          <div>
            <h2>Sensor Readings</h2>
            <p><strong>Temperature:</strong> {temperature} °C</p>
            <p><strong>Humidity:</strong> {humidity} %</p>
            <p><strong>Water Level:</strong> {waterLevel} meters</p>
            <p><strong>Turbidity:</strong> {turbidity} NTU</p>
          </div>
          <div style={{ display: "flex", flexDirection: "column", alignItems: "center" }}>
            <GaugeChart id="flood-probability-gauge" percent={floodProbability / 100} textColor="#000" />
          </div>
        </div>
        <div>
          <button onClick={openDam} style={{ padding: "10px 20px", fontSize: "16px", backgroundColor: "#007bff", color: "#fff", border: "none", borderRadius: "5px" }}>
            Open Dam
          </button>
        </div>
        <div>
          <h2>Inflow and Outflow Rates</h2>
          <Line data={lineChartData} options={{ responsive: true }} />
        </div>
      </div>
  );
};

export default App;
