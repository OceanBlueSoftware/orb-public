/**
 * @fileOverview TextTrack class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
hbbtv.objects.TextTrack = (function() {
    const prototype = Object.create(TextTrack.prototype);
    const privates = new WeakMap();
    const events = ['cuechange'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];
    const roProps = ['kind', 'label', 'language', 'activeCues', 'cues', 'id'];
    const props = ['isTTML', 'isEmbedded', 'inBandMetadataTrackDispatchType', 'renderingType'];
    const TRACK_MODE_SHOWING = 'showing';
    const TRACK_MODE_HIDDEN = 'hidden';
    const TRACK_MODE_DISABLED = 'disabled';

    for (const key of events) {
        Object.defineProperty(prototype, 'on' + key, {
            set(callback) {
                const p = privates.get(this);
                if (p['on' + key]) {
                    p.eventTarget.removeEventListener(key, p['on' + key]);
                }

                if (callback instanceof Object) {
                    p['on' + key] = callback;
                    if (callback) {
                        p.eventTarget.addEventListener(key, callback);
                    }
                } else {
                    p['on' + key] = null;
                }
            },
            get() {
                return privates.get(this)['on' + key];
            },
        });
    }

    for (const key of roProps) {
        Object.defineProperty(prototype, key, {
            get() {
                return privates.get(this).properties[key];
            },
        });
    }

    for (const key of props) {
        Object.defineProperty(prototype, key, {
            set(value) {
                privates.get(this).properties[key] = value;
            },
            get() {
                return privates.get(this).properties[key];
            },
        });
    }

    Object.defineProperty(prototype, 'mode', {
        set(value) {
            const p = privates.get(this);
            if (
                value !== p.properties.mode && [TRACK_MODE_DISABLED, TRACK_MODE_HIDDEN, TRACK_MODE_SHOWING].includes(value) &&
                hbbtv.bridge.configuration.getSubtitlesEnabled()
            ) {
                if (value !== TRACK_MODE_DISABLED) {
                    for (let track of p.mediaElement.textTracks) {
                        if (track.mode === TRACK_MODE_SHOWING && track !== p.trackProxy) {
                            track.mode = TRACK_MODE_HIDDEN;
                            break;
                        }
                    }
                    p.onTimeUpdate.apply(this);
                    p.mediaElement.addEventListener('timeupdate', p.onTimeUpdate, true);
                } else {
                    p.properties.activeCues.orb_clear();
                    p.mediaElement.removeEventListener('timeupdate', p.onTimeUpdate, true);
                }
                console.log("Updated mode property value to", value);
                p.properties.mode = value;
                p.mediaElement.textTracks.dispatchEvent(new Event('change'));
            }
        },
        get() {
            return privates.get(this).properties.mode;
        },
    });

    Object.defineProperty(prototype, 'default', {
        set(value) {
            privates.get(this).properties.default = !!value;
            if (value) {
                this.mode = TRACK_MODE_SHOWING;
            }
        },
        get() {
            return privates.get(this).properties.default;
        },
    });

    function makeEventTargetMethod(name) {
        return function() {
            EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
        };
    }

    for (const func of evtTargetMethods) {
        prototype[func] = makeEventTargetMethod(func);
    }

    function resetCueUpdateTimeout(p) {
        if (p.properties.onEnterExitTimeout) {
            clearTimeout(p.properties.onEnterExitTimeout);
            p.properties.onEnterExitTimeout = null;
        }
        p.properties.onEnterExitCurrentTime = -1;
    }

    prototype.addCue = function(cue) {
        const p = privates.get(this);
        resetCueUpdateTimeout(p);
        p.properties.cues.orb_addCue(cue);
        updateCues(this);
    };

    prototype.removeCue = function(cue) {
        const p = privates.get(this);
        resetCueUpdateTimeout(p);
        p.properties.cues.orb_removeCue(cue);
        p.properties.activeCues.orb_removeCue(cue);
        updateCues(this);
    };

    prototype.orb_finalize = function() {
        const p = privates.get(this);
        resetCueUpdateTimeout(p);
        p.proxy.unregisterObserver(p.observerId);
        p.activeCues.orb_clear();
        p.cues.orb_clear();
        p.mediaElement.removeEventListener('timeupdate', p.onTimeUpdate, true);
        privates.delete(this);
    };

    function updateCues(thiz) {
        const p = privates.get(thiz);
        const time = p.mediaElement.currentTime;
        resetCueUpdateTimeout(p);
        if (p.properties.cues.length === 0) {
            return;
        }
        let changed = false;
        let nextEventTimestamp = Number.MAX_SAFE_INTEGER;
        let nextEventCurrentTime;

        /* Detect cues that need to be displayed and calculate next onenter event */
        for (const cue of p.properties.cues) {
            /* Early for loop exit possible ??? */
            if (
                cue.endTime >= time &&
                cue.startTime <= time &&
                p.properties.activeCues.orb_indexOf(cue) < 0
            ) {
                p.properties.activeCues.orb_addCue(cue);
                if (p.properties.isIframe && (typeof cue.onenter === 'function')) {
                    cue.onenter();
                }
                changed = true;
            } else if (p.properties.isIframe && (cue.startTime > time)) {/* just relevant inside iframe */
                const onEnterTimestamp = cue.startTime - time;
                if (onEnterTimestamp < nextEventTimestamp) {
                    nextEventTimestamp = onEnterTimestamp;
                    nextEventCurrentTime = cue.startTime;
                }
            }
        }

        /* Detect activeCues that need to be removed and calculate next onexit event */
        for (let i = p.properties.activeCues.length - 1; i >= 0; --i) {
            const cue = p.properties.activeCues[i];
            if (cue.endTime < time || cue.startTime > time) {
                p.properties.activeCues.orb_removeCueAt(i);
                if (p.properties.isIframe && (typeof cue.onexit === 'function')) {
                    cue.onexit();
                }
                changed = true;
            } else if (p.properties.isIframe) { /* just relevant inside iframe */
                const onExitTimestamp = cue.endTime - time;
                if (onExitTimestamp < nextEventTimestamp) {
                    nextEventTimestamp = onExitTimestamp;
                    nextEventCurrentTime = cue.endTime;
                }
            }
        }

        if (changed) {
            thiz.dispatchEvent(new Event('cuechange'));
        }
        if (nextEventTimestamp !== Number.MAX_SAFE_INTEGER) {
            p.properties.onEnterExitCurrentTime = nextEventCurrentTime;
            p.properties.onEnterExitTimeout = setTimeout(updateCues, nextEventTimestamp * 1000, thiz);
        }
    }

    function initialise(mediaElement, proxy, id, kind, label, language, mode) {
        const properties = {
            id,
            kind,
            label,
            language,
            renderingType: 'html',
            cues: hbbtv.objects.createTextTrackCueList(),
            activeCues: hbbtv.objects.createTextTrackCueList(),
            mode: TRACK_MODE_DISABLED,
            default: false,
            onEnterExitTimeout: null,
            onEnterExitCurrentTime: -1,
            isIframe: window.location.href.startsWith('orb://')
        };

        const observerId = 'TextTrack_' + id;
        proxy.registerObserver(observerId, this);

        // We create a new Proxy object which we return in order to avoid ping-pong calls
        // between the iframe and the main window when the user requests a property update
        // or a function call.
        const trackProxy = new Proxy(this, {
            get: (target, property) => {
                if (typeof target[property] === 'function') {
                    if (!evtTargetMethods.includes(property)) {
                        return function() {
                            let args = [];
                            if (property !== 'addCue') {
                                args = Array.from(arguments);
                            } else {
                                const cueObj = {};
                                for (const key in arguments[0]) {
                                    let cueProp = arguments[0][key];
                                    if (typeof cueProp !== 'function') {
                                        if (key === 'data' && cueProp) {
                                            cueProp = [...Object.keys(cueProp).map(k => cueProp[k])];
                                        }
                                        cueObj[key] = cueProp;
                                    }
                                }
                                args.push(cueObj);
                            }
                            proxy.callObserverMethod(observerId, property, args);
                            return target[property].apply(target, arguments);
                        };
                    }
                    return target[property].bind(target);
                }
                return target[property];
            },
            set: (target, property, value) => {
                if (typeof target[property] !== 'function') {
                    proxy.updateObserverProperties(observerId, {
                        [property]: value,
                    });
                }
                target[property] = value;
                return true;
            },
        });

        privates.set(this, {
            length: 0,
            initialized: true,
            eventTarget: document.createDocumentFragment(),
            onTimeUpdate: (e) => {
                updateCues(this);
            },
            properties,
            mediaElement,
            observerId,
            proxy,
            trackProxy,
        });

        mediaElement.addEventListener('seeked', () => updateCues(this), true);

        this.mode = mode;
        return trackProxy;
    }

    return {
        prototype,
        initialise,
    };
})();

hbbtv.objects.createTextTrack = function(mediaElement, proxy, index, kind, label, language, mode) {
    const track = Object.create(hbbtv.objects.TextTrack.prototype);
    return hbbtv.objects.TextTrack.initialise.call(
        track,
        mediaElement,
        proxy,
        index,
        kind,
        label,
        language,
        mode
    );
};