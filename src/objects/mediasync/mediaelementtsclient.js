/**
 * @fileOverview MediaElementTsClient class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementTsClient = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineGetterSetterProperties(prototype, {
        correlationTimestamp: {
            get() {
                return privates.get(this).correlationTimestamp;
            },
            set(val) {
                const p = privates.get(this);
                p.correlationTimestamp = val;
                onMasterMediaUpdated.call(this, {
                    data: {
                        contentTime: p.masterMediaObserver.contentTime,
                    },
                });
            },
        },
    });

    prototype.addEventListener = function(type, listener) {
        privates.get(this).eventTarget.addEventListener(type, listener);
    };

    prototype.removeEventListener = function(type, listener) {
        privates.get(this).eventTarget.removeEventListener(type, listener);
    };

    prototype.destroy = function() {
        const p = privates.get(this);
        p.masterMediaObserver.removeEventListener('MediaUpdated', p.onMasterMediaUpdated);
        p.masterMediaObserver.removeEventListener('Error', p.onFailureToPresentMedia);
        p.mediaObject.removeEventListener('ended', p.onFailureToPresentMedia);
        clearInterval(p.pollIntervalId);
        privates.delete(this);
    };

    function onMasterMediaUpdated(e) {
        const p = privates.get(this);
        if (p.mediaObject.readyState < 3) {
            if (p.canSyncWithMaster) {
                p.canSyncWithMaster = false;
                dispatchErrorEvent(p.eventTarget, 1); // insufficient buffer size (transient)
            }
        } else if (!isNaN(e.data.contentTime)) {
            const targetTime =
                e.data.contentTime +
                (p.correlationTimestamp.tlvOther - p.correlationTimestamp.tlvMaster);
            checkMediaSync.call(this, targetTime);
        }
    }

    function checkMediaSync(contentTime) {
        const p = privates.get(this);
        if (contentTime >= 0 && !p.mediaObject.ended) {
            if (Math.abs(contentTime - p.mediaObject.currentTime) > p.tolerance / 1000.0) {
                console.log('Synchronised tlvOther with tlvMaster');
                p.mediaObject.currentTime = contentTime;
            }
            if (!p.canSyncWithMaster) {
                p.canSyncWithMaster = true;
                p.eventTarget.dispatchEvent(new Event('SyncNowAchievable'));
            }

            if (p.masterMediaObserver.timelineSpeedMultiplier == 0) {
                !p.mediaObject.paused && p.mediaObject.pause();
            } else {
                p.mediaObject.paused && p.mediaObject.play();
                p.mediaObject.playbackRate = p.masterMediaObserver.timelineSpeedMultiplier;
            }
        } else {
            !p.mediaObject.paused && p.mediaObject.pause();
            if (p.canSyncWithMaster) {
                p.canSyncWithMaster = false;
                dispatchErrorEvent(p.eventTarget, 11); // failed to synchronise media (transient)
            }
        }
    }

    function onFailureToPresentMedia() {
        const p = privates.get(this);
        if (p.canSyncWithMaster) {
            p.canSyncWithMaster = false;
            dispatchErrorEvent(p.eventTarget, 2); // failed to present media (transient)
        }
    }

    function dispatchErrorEvent(eventTarget, errorCode) {
        let evt = new Event('Error');
        evt.errorCode = errorCode;
        eventTarget.dispatchEvent(evt);
    }

    function initialise(
        mediaObject,
        correlationTimestamp,
        tolerance,
        multiDecoderMode,
        masterMediaObserver
    ) {
        privates.set(this, {
            mediaObject: mediaObject,
            tolerance: tolerance,
            multiDecoderMode: multiDecoderMode,
            masterMediaObserver: masterMediaObserver,
            canSyncWithMaster: true,
            correlationTimestamp: correlationTimestamp,
            eventTarget: document.createDocumentFragment(),
            onMasterMediaUpdated: onMasterMediaUpdated.bind(this),
            onFailureToPresentMedia: onFailureToPresentMedia.bind(this),
            pollIntervalId: setInterval(() => {
                checkMediaSync.call(this, masterMediaObserver.contentTime);
            }, 2000),
        });

        const p = privates.get(this);

        masterMediaObserver.addEventListener('MediaUpdated', p.onMasterMediaUpdated);
        masterMediaObserver.addEventListener('Error', p.onFailureToPresentMedia);
        mediaObject.addEventListener('ended', p.onFailureToPresentMedia);
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createMediaElementTsClient = function(
    mediaObject,
    timelineSelector,
    correlationTimestamp,
    tolerance,
    multiDecoderMode,
    masterMediaObserver
) {
    const client = Object.create(hbbtv.objects.MediaElementTsClient.prototype);
    hbbtv.objects.MediaElementTsClient.initialise.call(
        client,
        mediaObject,
        timelineSelector,
        correlationTimestamp,
        tolerance,
        multiDecoderMode,
        masterMediaObserver
    );
    return client;
};