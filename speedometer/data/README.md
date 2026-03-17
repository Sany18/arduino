# Web Server Data Folder

This folder contains static files served by the ESP32-S3 web server.

## Structure

```
data/
├── index.html      # Main page
├── style.css       # Styles
├── app.js          # JavaScript
└── images/         # (create this folder for images)
```

## How to Upload Files to ESP32

1. **Install PlatformIO filesystem uploader** (if not already installed)
   
2. **Upload filesystem to ESP32:**
   ```bash
   platformio run --target uploadfs
   ```

3. **The files will be uploaded to SPIFFS on your ESP32-S3**

## Supported File Types

- **HTML**: `.html`
- **CSS**: `.css`
- **JavaScript**: `.js`
- **JSON**: `.json`
- **Images**: `.png`, `.jpg`, `.jpeg`, `.gif`, `.svg`, `.ico`
- **Others**: `.xml`, `.pdf`, `.zip`

## API Endpoints

The web server also supports API endpoints:

- `GET /api/status` - Returns server status and uptime
- `POST /api/reset` - Reset servos (to be implemented)

## Adding New Files

1. Add your files to the `data/` folder
2. Run `platformio run --target uploadfs`
3. Access files via: `http://<ESP32-IP>/<filename>`

## Example URLs

- `http://192.168.1.100/` - Main page (index.html)
- `http://192.168.1.100/style.css` - CSS file
- `http://192.168.1.100/app.js` - JavaScript file
- `http://192.168.1.100/api/status` - API status endpoint
