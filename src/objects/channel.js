/**
 * @fileOverview OIPF Channel class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channel-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.Channel = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineConstantProperties(prototype, {
        TYPE_TV: 0,
        TYPE_RADIO: 1,
        TYPE_OTHER: 2,
        TYPE_ALL: 128,
        TYPE_HBBTV_DATA: 256,
        ID_ANALOG: 0,
        ID_DVB_C: 10,
        ID_DVB_S: 11,
        ID_DVB_T: 12,
        ID_DVB_SI_DIRECT: 13,
        ID_DVB_C2: 14,
        ID_DVB_S2: 15,
        ID_DVB_T2: 16,
        ID_ISDB_C: 20,
        ID_ISDB_S: 21,
        ID_ISDB_T: 22,
        ID_ATSC_T: 30,
        ID_IPTV_SDS: 40,
        ID_IPTV_URI: 41,
    });

    hbbtv.utils.defineGetterProperties(prototype, {
        channelType() {
            const p = privates.get(this);
            return p.channelData.channelType;
        },
        ccid() {
            const p = privates.get(this);
            return p.channelData.ccid;
        },
        dsd() {
            const p = privates.get(this);
            return p.channelData.dsd;
        },
        idType() {
            const p = privates.get(this);
            return p.channelData.idType;
        },
        nid() {
            const p = privates.get(this);
            return p.channelData.nid;
        },
        onid() {
            const p = privates.get(this);
            return p.channelData.onid;
        },
        tsid() {
            const p = privates.get(this);
            return p.channelData.tsid;
        },
        sid() {
            const p = privates.get(this);
            return p.channelData.sid;
        },
        name() {
            const p = privates.get(this);
            return p.channelData.name;
        },
        majorChannel() {
            const p = privates.get(this);
            return p.channelData.majorChannel;
        },
        terminalChannel() {
            const p = privates.get(this);
            return p.channelData.terminalChannel;
        },
        sourceID() {
            const p = privates.get(this);
            return p.channelData.sourceID;
        },
        ipBroadcastID() {
            const p = privates.get(this);
            return p.channelData.ipBroadcastID;
        },
    });

    // Initialise an instance of prototype
    function initialise(channelData) {
        privates.set(this, {});
        const p = privates.get(this);
        p.channelData = channelData; // Hold reference to caller's object
    }

    // Private method to get a copy of the channel data
    function cloneChannelData() {
        return Object.assign({}, privates.get(this).channelData);
    }

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).channelData);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        cloneChannelData: cloneChannelData,
    };
})();

hbbtv.objects.createChannel = function(channelData) {
    // Create new instance of hbbtv.objects.Channel.prototype
    const channel = Object.create(hbbtv.objects.Channel.prototype);
    hbbtv.objects.Channel.initialise.call(channel, channelData);
    return channel;
};