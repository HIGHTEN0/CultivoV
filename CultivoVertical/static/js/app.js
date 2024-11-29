const socket = io(); // Establece la conexión con el servidor

// Recibe los datos y las alarmas del backend
socket.on('nuevos_datos', function(data) {
    // Actualiza los datos en el frontend
    document.getElementById("humedad").innerText = `Humedad: ${data.humedad} %`;
    document.getElementById("temperatura").innerText = `Temperatura: ${data.temperatura} °C`;
    document.getElementById("nivelAgua").innerText = `Nivel de Agua: ${data.nivelAgua} cm`;
    
    // Muestra las alarmas activadas
    document.getElementById("alarmaNivelAgua").innerText = data.alarmas.NivelAguaBajo ? "Alarma: Nivel de Agua Bajo" : "";
    document.getElementById("alarmaTempElevada").innerText = data.alarmas.TemperaturaElevada ? "Alarma: Temperatura Elevada" : "";
    document.getElementById("alarmaHumedadBaja").innerText = data.alarmas.HumedadBaja ? "Alarma: Humedad Baja" : "";
    document.getElementById("alarmaHumedadAlta").innerText = data.alarmas.HumedadAlta ? "Alarma: Humedad Alta" : "";
});
