// Fetch and display status
async function refreshStatus() {
  try {
    const response = await fetch('/api/status');
    const data = await response.json();
    
    if (data.status === 'ok') {
      document.getElementById('server-status').textContent = 'Online';
      document.getElementById('server-status').style.color = '#28a745';
      document.getElementById('uptime').textContent = (data.uptime / 1000).toFixed(2) + ' seconds';
    }
  } catch (error) {
    console.error('Error fetching status:', error);
    document.getElementById('server-status').textContent = 'Error';
    document.getElementById('server-status').style.color = '#dc3545';
  }
}

// Auto-refresh status every 5 seconds
setInterval(refreshStatus, 5000);

// Clock variables
let localTime = 0;  // Current time in seconds since Unix epoch
let lastSyncTime = 0;  // millis() when last synced

// Sync time with browser
async function syncTime() {
  try {
    const now = new Date();
    const utcTime = Math.floor(now.getTime() / 1000);  // UTC timestamp in seconds
    const timezoneOffset = now.getTimezoneOffset() * 60;  // Offset in seconds (negative for ahead of UTC)
    const adjustedTime = utcTime - timezoneOffset;  // Adjust for timezone
    
    console.log('Syncing time to ESP32:', now.toLocaleTimeString(), 'UTC offset:', timezoneOffset / 3600, 'hours');
    
    const response = await fetch('/api/time', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ time: adjustedTime })
    });
    
    console.log('Sync response status:', response.status);
    const result = await response.json();
    console.log('Sync response:', result);
    
    if (response.ok) {
      localTime = utcTime;
      lastSyncTime = Date.now();
      console.log('Time synced successfully!');
    } else {
      console.error('Time sync failed with status:', response.status);
    }
  } catch (error) {
    console.error('Error syncing time:', error);
  }
}

// Update clock display
function updateClock() {
  if (localTime > 0) {
    // Calculate elapsed time since last sync
    const elapsed = Math.floor((Date.now() - lastSyncTime) / 1000);
    const currentTime = localTime + elapsed;
    
    // Format time as HH:MM:SS
    const date = new Date(currentTime * 1000);
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    const seconds = String(date.getSeconds()).padStart(2, '0');
    
    document.getElementById('clock-display').textContent = `${hours}:${minutes}:${seconds}`;
  }
}

// Sync time from ESP32 (get more precise time)
async function syncFromESP32() {
  try {
    const response = await fetch('/api/time');
    const data = await response.json();
    
    if (data.time > 0) {
      localTime = data.time;
      lastSyncTime = Date.now();
    }
  } catch (error) {
    console.error('Error fetching time from ESP32:', error);
  }
}

// Update clock every second
setInterval(updateClock, 1000);

// Sync from ESP32 every 30 seconds for more precise time
setInterval(syncFromESP32, 30000);

// Initial sync on page load
syncTime();

// Initial status check
refreshStatus();
