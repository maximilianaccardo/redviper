const char INDEX_HTML[] PROGMEM = R"===(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Red Viper Control</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 20px; }
    input, select { padding: 10px; margin: 10px; font-size: 16px; }
    button { padding: 10px 20px; font-size: 16px; margin: 5px; cursor: pointer; }
    .status-container { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; margin-bottom: 20px; }
    .motor-status { border: 1px solid #ccc; padding: 10px; border-radius: 5px; min-width: 80px; }
    .selected { background-color: #e0f7fa; border-color: #00acc1; }
    .controls { border-top: 1px solid #eee; padding-top: 20px; }
  </style>
</head>
<body>
  <h1>Red Viper Control Panel</h1>
  
  <div class='status-container' id='status-container'>
    <!-- Motor statuses will be populated here -->
    Loading motors...
  </div>

  <div class='controls'>
    <label for='motor-select'>Selected Motor:</label>
    <select id='motor-select' onchange='updateSelection()'>
      <option value='0'>Motor 0</option>
      <option value='1'>Motor 1</option>
      <option value='2'>Motor 2</option>
      <option value='3'>Motor 3</option>
      <option value='4'>Motor 4</option>
    </select>

    <div>
      <input type='number' id='degrees' placeholder='Enter degrees' value='90'>
      <button onclick='moveMotor()'>Move</button>
    </div>
    <div>
      <button onclick='moveDegrees(90)'>+90</button>
      <button onclick='moveDegrees(-90)'>-90</button>
      <button onclick='moveDegrees(180)'>+180</button>
      <button onclick='moveDegrees(-180)'>-180</button>
    </div>
    
    <div style='margin-top: 20px; border-top: 1px solid #ccc; padding-top: 20px;'>
      <h3>Calibration</h3>
      <button onclick='setZero()' style='background: #4CAF50; color: white;'>Set Zero Position</button>
      <div style='margin-top: 10px;'>
        <input type='number' id='setpos' placeholder='Set position (degrees)' style='width: 150px;'>
        <button onclick='setPosition()'>Set Position</button>
      </div>
    </div>
  </div>
  
  <div id='log' style='margin-top:20px; color: #666;'></div>
  
  <script>
    let currentMotor = 0;

    function updateSelection() {
      currentMotor = document.getElementById('motor-select').value;
      // Highlight selected motor in status view
      document.querySelectorAll('.motor-status').forEach(el => {
        el.classList.remove('selected');
        if(el.id === 'motor-status-' + currentMotor) el.classList.add('selected');
      });
    }

    function log(msg) {
      document.getElementById('log').innerText = msg;
    }

    function moveMotor() {
      var deg = document.getElementById('degrees').value;
      moveDegrees(deg);
    }
    
    function moveDegrees(deg) {
      log('Moving Motor ' + currentMotor + '...');
      fetch('/move?motor=' + currentMotor + '&degrees=' + deg)
        .then(response => response.text())
        .then(data => {
          log(data);
          updateStatus();
        });
    }
    
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const container = document.getElementById('status-container');
          let html = '';
          data.motors.forEach(m => {
            const isSelected = m.id == currentMotor ? ' selected' : '';
            html += `<div id='motor-status-${m.id}' class='motor-status${isSelected}' onclick='selectMotor(${m.id})'>
              <strong>Motor ${m.id}</strong><br>
              ${m.degrees}&deg;
            </div>`;
          });
          container.innerHTML = html;
          
          // Ensure dropdown matches if changed externally or initially
          document.getElementById('motor-select').value = currentMotor;
        })
        .catch(err => console.error('Status update failed', err));
    }
    
    function selectMotor(id) {
      currentMotor = id;
      updateStatus(); // Refresh UI selection
    }

    function setZero() {
      log('Setting zero for Motor ' + currentMotor + '...');
      fetch('/zero?motor=' + currentMotor)
        .then(response => response.text())
        .then(data => {
          log(data);
          updateStatus();
        });
    }
    
    function setPosition() {
      var degrees = document.getElementById('setpos').value;
      if (degrees === '') {
        log('Please enter a position');
        return;
      }
      var steps = Math.round(degrees * 2048 / 360.0);
      log('Setting position for Motor ' + currentMotor + '...');
      fetch('/setpos?motor=' + currentMotor + '&position=' + steps)
        .then(response => response.text())
        .then(data => {
          log(data);
          updateStatus();
        });
    }
    
    // Update status immediately on load and then every 2 seconds
    updateStatus();
    setInterval(updateStatus, 2000);
  </script>
</body>
</html>
)===";
