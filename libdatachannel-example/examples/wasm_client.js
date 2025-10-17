// This file contains the example client for WebRTC communication using WebAssembly.
// It initializes the RTC connection and handles data transmission in a web environment.

const { RTCPeerConnection, RTCSessionDescription } = window;

let peerConnection;
let dataChannel;

function startConnection() {
    peerConnection = new RTCPeerConnection();

    // Create a data channel
    dataChannel = peerConnection.createDataChannel("chat");

    // Set up event handlers
    dataChannel.onopen = () => {
        console.log("Data channel is open");
        dataChannel.send("Hello from WebAssembly client!");
    };

    dataChannel.onmessage = (event) => {
        console.log("Message from server:", event.data);
    };

    // Handle ICE candidates
    peerConnection.onicecandidate = (event) => {
        if (event.candidate) {
            // Send the candidate to the remote peer
            console.log("New ICE candidate:", event.candidate);
        }
    };

    // Create an offer and set local description
    peerConnection.createOffer().then((offer) => {
        return peerConnection.setLocalDescription(offer);
    }).then(() => {
        // Send the offer to the remote peer
        console.log("Offer sent to remote peer:", peerConnection.localDescription);
    }).catch((error) => {
        console.error("Error creating offer:", error);
    });
}

// Start the connection when the page loads
window.onload = startConnection;