import React, { useState, useEffect } from 'react';
import axios from 'axios';
import 'bootstrap/dist/css/bootstrap.min.css';

const App = () => {
  const [sensorData, setSensorData] = useState([]);
  const [error, setError] = useState(null);

  // Fetch data from the server every 5 seconds
  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await axios.get('http://localhost:3000/api/data');
        setSensorData(response.data);
      } catch (err) {
        setError('Error fetching data');
      }
    };

    fetchData();
    const intervalId = setInterval(fetchData, 5000); // Fetch every 5 seconds

    // Cleanup on unmount
    return () => clearInterval(intervalId);
  }, []);

  return (
    <div className="container mt-5">
      <h1 className="text-center mb-4">Health Report</h1>

      {error && <div className="alert alert-danger">{error}</div>}

      <div className="card">
        <div className="card-header">
          Sensor Data
        </div>
        <div className="card-body">
          <table className="table table-bordered table-striped">
            <thead>
              <tr>
                <th>Light Intensity</th>
                <th>Temperature (°F)</th>
                <th>Sleep Score</th>
                <th>Circadian Rhythm Score</th>
                <th>Movement</th>
                <th>Timestamp</th>
              </tr>
            </thead>
            <tbody>
              {sensorData.length === 0 ? (
                <tr>
                  <td colSpan="6" className="text-center">No data available</td>
                </tr>
              ) : (
                sensorData.map((data, index) => (
                  <tr key={index}>
                    <td>{data.lightIntensity}</td>
                    <td>{data.temperatureF}°F</td>
                    <td>{data.sleepScore}</td>
                    <td>{data.circadianRhythmScore}</td>
                    <td>{data.movement ? "Movement Detected" : "No Movement"}</td>
                    <td>{new Date(data.timestamp).toLocaleString()}</td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};

export default App;
