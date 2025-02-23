const express = require('express');
const bodyParser = require('body-parser');

const app = express();
const port = 3000;

// Middleware to parse JSON data
app.use(bodyParser.json());

// Store sensor data in-memory
let sensorData = [];

// Endpoint to receive data from ESP32
app.post('/api/upload', (req, res) => {
  const { lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement } = req.body;
  console.log('Received data:', { lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement });

  // Save data to the in-memory store
  sensorData.push({ lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement, timestamp: new Date() });

  // Respond with success message
  res.status(200).send({ status: 'success', message: 'Data received' });
});

// Endpoint to retrieve stored sensor data
app.get('/api/data', (req, res) => {
  res.status(200).json(sensorData);
});

// Start the server
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});