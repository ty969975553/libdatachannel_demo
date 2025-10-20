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

const wasmBaseUrl = new URL('./wasm/', import.meta.url);

let wasmModule = null;
let answerSession = null;
let pollTimer = null;
let lastAnswerText = '';

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

function updateOfferStatus(text, isError = false) {
    offerStatus.textContent = text;
    offerStatus.classList.toggle('error', isError);
}

function updateDescriptionStatus(text, isError = false) {
    const badge = descriptionStatus.querySelector('.badge');
    badge.textContent = text;
    badge.classList.toggle('error', isError);
}

function updateCandidateStatus(count) {
    const badge = candidateStatus.querySelector('.badge');
    badge.textContent = `${count}`;
}

function parseCandidateCount(signalText) {
    if (!signalText) {
        return 0;
    }
    let count = 0;
    const lines = signalText.split(/\r?\n/);
    for (const line of lines) {
        if (line.startsWith('candidate:')) {
            count += 1;
        }
    }
    return count;
}

function updateAnswerOutput(text) {
    if (text === lastAnswerText) {
        return;
    }
    lastAnswerText = text;
    answerOutput.value = text;
    updateCandidateStatus(parseCandidateCount(text));
    if (text) {
        updateDescriptionStatus('updated – copy to answer.txt');
    }
}

function injectWasmScript(url) {
    return new Promise((resolve, reject) => {
        const script = document.createElement('script');
        script.src = url.toString();
        script.async = true;
        script.onload = () => resolve();
        script.onerror = () => reject(new Error(`Failed to load ${url}`));
        document.head.appendChild(script);
    });
}

async function loadWasmModule() {
    try {
        const moduleFactory = (await import('./wasm/wasm_app.js')).default;
        return moduleFactory({
            locateFile: (path) => new URL(path, wasmBaseUrl).toString(),
        });
    } catch (error) {
        console.error('Failed to import WASM module', error);
        try {
            await injectWasmScript(new URL('./wasm/wasm_app.js', import.meta.url));
        } catch (loadError) {
            console.error(loadError);
            throw error;
        }
        if (window.LibDataChannelExample) {
            return window.LibDataChannelExample({
                locateFile: (path) => new URL(path, wasmBaseUrl).toString(),
            });
        }
        throw error;
    }
}

function updateChannelState(isOpen) {
    messageInput.disabled = !isOpen;
    sendMessageButton.disabled = !isOpen;
}

function drainSessionQueues() {
    if (!answerSession) {
        return;
    }

    for (const event of answerSession.drainEvents()) {
        logEvent(event);
    }

    for (const message of answerSession.drainMessages()) {
        logMessage('Native', message);
    }

    if (answerSession.hasSignalUpdate()) {
        const signalText = answerSession.consumeSignal();
        if (signalText) {
            updateAnswerOutput(signalText);
            logEvent('Answer updated. Copy to answer.txt to continue signaling.');
        }
    }

    updateChannelState(answerSession.isChannelOpen());
}

async function initializeWasm() {
    updateOfferStatus('Loading libdatachannel WASM…');
    try {
        wasmModule = await loadWasmModule();
        answerSession = new wasmModule.WasmAnswerSession();
        if (!answerSession.initialize()) {
            updateOfferStatus('Failed to initialize WASM session', true);
            logEvent('WASM session initialization failed.');
            return;
        }
        logEvent('libdatachannel WASM session ready. Paste the offer and click "Apply Offer".');
        updateOfferStatus('Ready for offer');
        pollTimer = window.setInterval(drainSessionQueues, 250);
        drainSessionQueues();
    } catch (error) {
        console.error(error);
        updateOfferStatus('Unable to load libdatachannel WASM runtime', true);
        logEvent(`Failed to load WASM runtime: ${error.message}`);
    }
}

async function applyOffer() {
    if (!answerSession) {
        updateOfferStatus('WASM session not ready', true);
        return;
    }

    const offerText = offerInput.value.trim();
    if (!offerText) {
        updateOfferStatus('Offer missing', true);
        return;
    }

    updateOfferStatus('Applying offer…');
    updateDescriptionStatus('processing');

    const result = answerSession.applyOffer(offerText);
    const success = Boolean(result && result.success);
    if (!success) {
        const errorMessage = result && result.error ? result.error : 'Failed to apply offer';
        updateOfferStatus(errorMessage, true);
        updateDescriptionStatus('error', true);
        logEvent(`Error applying offer: ${errorMessage}`);
        return;
    }

    updateOfferStatus('Offer applied');
    logEvent('Offer applied. Waiting for local answer from libdatachannel…');
    drainSessionQueues();
}

function sendBrowserMessage() {
    if (!answerSession || !answerSession.isChannelOpen()) {
        logEvent('Data channel not open yet.');
        return;
    }

    const message = messageInput.value.trim();
    if (!message) {
        return;
    }

    if (!answerSession.sendMessage(message)) {
        logEvent('Failed to send message through libdatachannel.');
        return;
    }

    logMessage('Browser', message);
    messageInput.value = '';
}

applyOfferButton.addEventListener('click', () => {
    applyOffer();
});

sendMessageButton.addEventListener('click', () => {
    sendBrowserMessage();
});

messageInput.addEventListener('keydown', (event) => {
    if (event.key === 'Enter' && !event.shiftKey) {
        event.preventDefault();
        sendBrowserMessage();
    }
});

initializeWasm();

logEvent('Initializing libdatachannel WASM answer…');
updateChannelState(false);

