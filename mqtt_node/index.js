#!/usr/bin/env node
const mqtt = require("mqtt");
//const client = mqtt.connect("mqtt://localhost");
const client = mqtt.connect("mqtt://test.mosquitto.org");

client.on("connect", () => {
  client.subscribe("roo-sensors", (err) => {
    if (err) {
			console.error(err);
			return;
		}
  });
});

client.on("message", (topic, message) => {
  // message is Buffer
  console.log(
		topic.toString(), message, '\n',
		JSON.stringify(message.toJSON().data, null, null), '\n',
		JSON.stringify({
			crc_b1: message[0],
			crc_b2: message[1],
			adc0: message[2],
			adc1: message[3],
			adc2: message[4],
			adc3: message[5],
			adc4: message[6],
			adc5: message[7],
			adc6: message[8],
			adc7: message[9],
			adc8: message[10],
			gpio: `0b${message[11].toString(2).padStart(8, '0')}`,
		}),
	);
});

function cleanup() {
	console.info('Closing application');
  client.end();
	process.exit(0);
}

process.on('SIGINT', cleanup);
process.on('SIGTERM', cleanup);
