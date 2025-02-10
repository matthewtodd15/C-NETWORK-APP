console.log("Establishing websocket connection...");
const websocket = new WebSocket("ws://localhost:4242/ws");

// Must send after connections are open
websocket.onopen = (event) => {
  console.log("Websocket successfully connected");
  //websocket.send("Here's some text that the server is urgently awaiting!");
};

websocket.onmessage = (event) => {
  console.log("Message received!");
  console.log(event.data);
};

websocket.onerror = (event) => {
  console.log("Connection error");
  console.log(event);
};

websocket.onclose = (event) => {
  console.log("Connection closed");
  console.log(event);
};
