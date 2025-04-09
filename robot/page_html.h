
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PID Controller</title>
    <style>
        body {
            margin: 0;
            background-color: black;
            color: greenyellow;
            font-family: monospace;
            display: flex;
            height: 100vh;
            overflow: hidden;
        }
        #jsonOutput {
            flex: 1;
            padding: 20px;
            white-space: pre-wrap;
            word-wrap: break-word;
            overflow: auto;
        }
        #controls {
            width: 50vw;
            padding: 20px;
            background: #222;
            color: white;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        label, button {
            font-size: 16px;
        }
        input {
            width: 100%;
        }
        .slider-value {
            display: inline-block;
            width: 40px;
            text-align: right;
        }
    </style>
</head>
<body>
    <pre id="jsonOutput">Loading...</pre>
    <div id="controls">
        <button onclick="setHome()">Set Home</button>

        <h3>PID</h3>
        <label for="pidIndex">Index:</label>
        <select id="pidIndex">
          <option value="0">angle</option>
          <option value="1">pos</option>
          <option value="2">turn</option>
        </select>
        <button onclick="fetchPID()">Get PID</button>
        <label>Kp: <input type="range" id="kp" min="0" max="20" step="0.0001" oninput="updateValue('kp_value', this.value)"><span id="kp_value" class="slider-value">0</span></label>
        <label>Ki: <input type="range" id="ki" min="0" max="120" step="0.0001" oninput="updateValue('ki_value', this.value)"><span id="ki_value" class="slider-value">0</span></label>
        <label>Kd: <input type="range" id="kd" min="0" max="0.3" step="0.0001" oninput="updateValue('kd_value', this.value)"><span id="kd_value" class="slider-value">0</span></label>
        <div>
            <label>Direction:</label>
            <label><input type="radio" name="direction" value="0" checked> Direct</label>
            <label><input type="radio" name="direction" value="1"> Reverse</label>
        </div>
        <button onclick="setPID()">Set PID</button>
        
        <label>Desired Yaw: <input type="range" id="gyro_kp" min="-180" max="180" step="5" oninput="updateValue('desired_yaw', this.value);setDesiredYaw(this.value)"><span id="desired_yaw" class="slider-value">0</span></label>
    </div>
    <script>
        async function fetchData(){
          try{
            const res = await fetch("get_stuff");
            document.getElementById("jsonOutput").textContent = JSON.stringify(await res.json(), null, 2);
          }catch(error){
            document.getElementById("jsonOutput").textContent = "Error: " + error.message;
          }
        }

        async function loopFetch(){
           while(true){
            await fetchData();  
           }
        }
        loopFetch();

        document.getElementById("pidIndex").addEventListener("change", (event) => {
          fetchPID();
        });

        async function fetchPID() {
            const index = parseInt(document.getElementById("pidIndex").value);
            
            const res = await fetch("/get_pid", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({index})
            });
            const data = await res.json();
            document.getElementById("kp").value = data.kp;
            document.getElementById("ki").value = data.ki;
            document.getElementById("kd").value = data.kd;
            updateValue('kp_value', data.kp);
            updateValue('ki_value', data.ki);
            updateValue('kd_value', data.kd);
            document.querySelector(`input[name="direction"][value="${data.direction}"]`).checked = true;
        }
        window.onload = () => {
          fetchPID(); 
        }

        async function setPID() {
            const index = parseInt(document.getElementById("pidIndex").value);

            const direction = document.querySelector('input[name="direction"]:checked').value;
            const data = {
                index,
                kp: parseFloat(document.getElementById("kp").value),
                ki: parseFloat(document.getElementById("ki").value),
                kd: parseFloat(document.getElementById("kd").value),
                direction: parseInt(direction)
            };
            await fetch("/set_pid", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(data)
            });
        }

        function setDesiredYaw(yaw){
          fetch("/set_desired_yaw", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({yaw})
            });
        }

        function setGyroPID() {
            const direction = document.querySelector('input[name="gyro_direction"]:checked').value;
            const data = {
                kp: parseFloat(document.getElementById("gyro_kp").value),
                ki: parseFloat(document.getElementById("gyro_ki").value),
                kd: parseFloat(document.getElementById("gyro_kd").value),
                direction: parseInt(direction)
            };
            fetch("/set_gyro_pid", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(data)
            });
        }

        function updateValue(id, value) {
            document.getElementById(id).textContent = value;
        }
    </script>
</body>
</html>
)rawliteral";
