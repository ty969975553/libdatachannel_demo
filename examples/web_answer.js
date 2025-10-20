const offerInput = document.getElementById('offerInput');
const answerOutput = document.getElementById('answerOutput');
const applyOfferButton = document.getElementById('applyOffer');
const offerStatus = document.getElementById('offerStatus');
const descriptionStatus = document.getElementById('descriptionStatus');
const candidateStatus = document.getElementById('candidateStatus');
const eventLog = document.getElementById('eventLog');
const messageLog = document.getElementById('messageLog');
const messageInput = document.getElementById('messageInput');
const sendMessageButton = document.getElementById('sendMessage');

let peerConnection;
let dataChannel;
let hasRemoteDescription = false;
const localCandidates = [];
const appliedCandidates = new Set();

function logEvent(message) {
    const timestamp = new Date().toLocaleTimeString();
    eventLog.textContent += `[${timestamp}] ${message}\n`;
    eventLog.scrollTop = eventLog.scrollHeight;
}

function logMessage(prefix, message) {
    const timestamp = new Date().toLocaleTimeString();
    messageLog.textContent += `[${timestamp}] ${prefix}: ${message}\n`;
    messageLog.scrollTop = messageLog.scrollHeight;
}

function updateDescriptionStatus(state, isError = false) {
    const badge = descriptionStatus.querySelector('.badge');
    badge.textContent = state;
    badge.classList.toggle('error', isError);
}

function updateCandidateStatus() {
    const badge = candidateStatus.querySelector('.badge');
    badge.textContent = `${localCandidates.length}`;
}

function buildSignalText() {
    if (!peerConnection || !peerConnection.localDescription) {
        return '';
    }

    const lines = [];
    lines.push('type:answer');
    lines.push('sdp-begin');
    const sdp = peerConnection.localDescription.sdp.replace(/\r\n/g, '\n').trimEnd();
    lines.push(sdp);
    if (sdp.length > 0 && !sdp.endsWith('\n')) {
        lines.push('');
    }
    lines.push('sdp-end');

    for (const candidate of localCandidates) {
        lines.push(`candidate:${candidate.sdpMid}|${candidate.candidate}`);
    }

    return lines.join('\n');
}

function refreshAnswerOutput() {
    answerOutput.value = buildSignalText();
}

function ensurePeerConnection() {
    if (peerConnection) {
        return peerConnection;
    }

    peerConnection = new RTCPeerConnection({
        iceServers: [
            { urls: 'stun:stun.l.google.com:19302' }
        ]
    });

    peerConnection.onicecandidate = (event) => {
        if (event.candidate) {
            const candidateKey = `${event.candidate.sdpMid}|${event.candidate.candidate}`;
            const exists = localCandidates.some((entry) => `${entry.sdpMid}|${entry.candidate}` === candidateKey);
            if (!exists) {
                localCandidates.push({
                    sdpMid: event.candidate.sdpMid ?? '0',
                    candidate: event.candidate.candidate
                });
                updateCandidateStatus();
                refreshAnswerOutput();
            }
        } else {
            logEvent('Finished gathering local ICE candidates.');
        }
    };

    peerConnection.onconnectionstatechange = () => {
        logEvent(`Connection state changed: ${peerConnection.connectionState}`);
        if (peerConnection.connectionState === 'connected') {
            messageInput.disabled = false;
            sendMessageButton.disabled = false;
        }
    };

    peerConnection.oniceconnectionstatechange = () => {
        logEvent(`ICE connection state: ${peerConnection.iceConnectionState}`);
    };

    peerConnection.ondatachannel = (event) => {
        dataChannel = event.channel;
        logEvent(`Received data channel "${dataChannel.label}"`);
        attachDataChannelHandlers();
    };

    return peerConnection;
}

function attachDataChannelHandlers() {
    if (!dataChannel) {
        return;
    }

    dataChannel.onopen = () => {
        logEvent('Data channel is open.');
        messageInput.disabled = false;
        sendMessageButton.disabled = false;
    };

    dataChannel.onmessage = (event) => {
        logMessage('Native', event.data);
    };

    dataChannel.onclose = () => {
        logEvent('Data channel closed.');
        messageInput.disabled = true;
        sendMessageButton.disabled = true;
    };
}

function parseSignalText(text) {
    const result = {
        type: '',
        sdp: '',
        candidates: []
    };

    const lines = text.split(/\r?\n/);
    let index = 0;

    while (index < lines.length) {
        const line = lines[index].trim();
        if (line.length === 0) {
            index++;
            continue;
        }
        if (line.startsWith('type:')) {
            result.type = line.substring(5).trim();
        } else if (line === 'sdp-begin') {
            index++;
            const sdpLines = [];
            while (index < lines.length && lines[index] !== 'sdp-end') {
                sdpLines.push(lines[index]);
                index++;
            }
            result.sdp = sdpLines.join('\n');
        } else if (line.startsWith('candidate:')) {
            const payload = line.substring(10);
            const separator = payload.indexOf('|');
            if (separator === -1) {
                index++;
                continue;
            }
            const sdpMid = payload.substring(0, separator) || '0';
            const candidate = payload.substring(separator + 1);
            if (candidate) {
                result.candidates.push({ sdpMid, candidate });
            }
        }
        index++;
    }

    return result;
}

async function applyOffer() {
    const offerText = offerInput.value.trim();
    if (!offerText) {
        offerStatus.textContent = 'Offer missing';
        offerStatus.classList.add('error');
        return;
    }

    offerStatus.textContent = 'Processing offer…';
    offerStatus.classList.remove('error');

    const { type, sdp, candidates } = parseSignalText(offerText);
    if (type !== 'offer' || !sdp) {
        offerStatus.textContent = 'Invalid offer data';
        offerStatus.classList.add('error');
        return;
    }

    const pc = ensurePeerConnection();

    if (!hasRemoteDescription) {
        try {
            await pc.setRemoteDescription({ type: 'offer', sdp });
            hasRemoteDescription = true;
            offerStatus.textContent = 'Offer applied';
            logEvent('Remote description applied. Creating answer…');

            const answer = await pc.createAnswer();
            await pc.setLocalDescription(answer);
            updateDescriptionStatus('created – copy to answer.txt');
            refreshAnswerOutput();
            logEvent('Local answer created. Copy it to answer.txt.');
        } catch (error) {
            console.error(error);
            offerStatus.textContent = 'Failed to apply offer';
            offerStatus.classList.add('error');
            updateDescriptionStatus('error', true);
            return;
        }
    } else {
        offerStatus.textContent = 'Offer updated';
    }

    for (const entry of candidates) {
        const key = `${entry.sdpMid}|${entry.candidate}`;
        if (appliedCandidates.has(key)) {
            continue;
        }
        try {
            await pc.addIceCandidate({
                candidate: entry.candidate,
                sdpMid: entry.sdpMid,
                sdpMLineIndex: 0
            });
            appliedCandidates.add(key);
        } catch (error) {
            console.error('Failed to add remote candidate', error);
            logEvent(`Failed to add remote candidate: ${error.message}`);
        }
    }
}

applyOfferButton.addEventListener('click', () => {
    applyOffer();
});

sendMessageButton.addEventListener('click', () => {
    if (!dataChannel || dataChannel.readyState !== 'open') {
        logEvent('Data channel not open yet.');
        return;
    }
    const message = messageInput.value.trim();
    if (!message) {
        return;
    }
    dataChannel.send(message);
    logMessage('Browser', message);
    messageInput.value = '';
});

messageInput.addEventListener('keydown', (event) => {
    if (event.key === 'Enter' && !event.shiftKey) {
        event.preventDefault();
        sendMessageButton.click();
    }
});

logEvent('Ready. Paste the offer and click "Apply Offer".');
