hbbtv.objects.DASHEvent = (function() {
    const prototype = { };
    const privates = new WeakMap();

    hbbtv.utils.defineGetterProperties(prototype, {
        pauseOnExit() {
            return false;
        },
        id() {
            return privates.get(this).eventData.id || "";
        },
        startTime() {
            return privates.get(this).eventData.startTime || 0;
        },
        endTime() {
            if (privates.get(this).eventData.duration === 4294967295) {
                return Number.MAX_VALUE;
            }
            return privates.get(this).eventData.duration + this.startTime || Number.MAX_VALUE;
        },
        data() {
            return privates.get(this).eventData.data;
        }
    });

    // Initialise an instance of prototype
    function initialise(streamEvent) {
        privates.set(this, {
            eventData: streamEvent.DASHEvent,
        });
        const data = streamEvent.text;
        if (typeof(data) === "string") {
            if (streamEvent.DASHEvent.contentEncoding.startsWith("binaryHex")) {// === "binaryHex") {
                if (streamEvent.data.length % 2 !== 0) {
                    throw new Error("Hex string must have an even length");
                }
                const buffer = new ArrayBuffer(streamEvent.data.length / 2);
                const uint8Array = new Uint8Array(buffer);
                for (let i = 0; i < uint8Array.length; i++) {
                    uint8Array[i] = parseInt(streamEvent.data.substr(i * 2, 2), 16);
                }
                streamEvent.DASHEvent.data = buffer;
                streamEvent.text = String.fromCharCode(...uint8Array);
            } else if (streamEvent.DASHEvent.contentEncoding.startsWith("binary") || (data.length === 0)) {
                const textEncoder = new TextEncoder();
                streamEvent.DASHEvent.data = textEncoder.encode(data).buffer;
            }
            else {
                try {
                    const parser = new DOMParser();
                    streamEvent.DASHEvent.data = parser.parseFromString(data, 'text/xml');
                }
                catch(e) {
                    console.warn(e.message);
                }
            }
        }
        streamEvent.DASHEvent = this;
    }

    return {
        prototype: prototype,
        initialise: initialise
    };
})();

hbbtv.objects.createDASHEvent = function(streamEvent) {
    const obj = Object.create(hbbtv.objects.DASHEvent.prototype);
    hbbtv.objects.DASHEvent.initialise.call(obj, streamEvent);
    return obj;
};