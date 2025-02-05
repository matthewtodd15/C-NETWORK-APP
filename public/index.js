console.log("Establishing websocket connection...");
const websocket = new WebSocket("/ws");

// Must send after connections are open
websocket.onopen = (event) => {
	console.log("Websocket successfully connected");
	websocket.send("Here's some text that the server is urgently awaiting!");
};

websocket.onmessage = (event) => {
	console.log("Message received!");
	console.log(event);
}
