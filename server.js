const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const path = require('path');

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });

const app = express();
const port = 3000;

// Middleware
app.use(bodyParser.json());
app.use(cors());
app.use(express.static(path.join(__dirname, 'public')));  // Serve static files

// API Endpoint for ESP32 Data
let sensorData = [];

// WebSocket setup
wss.on('connection', ws => {
  console.log('Client connected');
  
  // Send existing sensor data to the newly connected client
  ws.send(JSON.stringify(sensorData));

  // You can add additional logic here to handle messages from the client, if necessary
});

// POST endpoint for uploading sensor data
app.post('/api/upload', (req, res) => {
  const { lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement } = req.body;
  console.log('Received data:', { lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement });

  // Add new sensor data to the array
  const newData = { lightIntensity, temperatureF, sleepScore, circadianRhythmScore, movement, timestamp: new Date() };
  sensorData.push(newData);

  // Notify all connected WebSocket clients with the updated data
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify([newData]));  // Send the new data as an array
    }
  });

  res.status(200).send({ status: 'success', message: 'Data received' });
});

// GET endpoint for retrieving all sensor data
app.get('/api/data', (req, res) => {
  res.status(200).json(sensorData);
});

// Catch-all route to serve index.html for any undefined routes (for Single Page Apps)
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start server
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
