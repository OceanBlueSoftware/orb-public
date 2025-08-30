// network.store.js
import { create } from 'zustand';
import { NetworkService } from './network.service.js';

const service = new NetworkService();

export const useNetworkStore = create((set, get) => ({
  ipAddress: null,
  currentNetworkInterface: null,
  isConnectedToInternet: false,
  lastError: null,
  _started: false,

  start: async () => {
    if (get()._started) return;
    set({ _started: true });

    service.subscribe((evt) => {
      switch (evt.type) {
        case 'IP_CHANGED':
          set({
            ipAddress: evt.ip,
            currentNetworkInterface: evt.name,
          });
          break;
        case 'STATUS':
          set({ isConnectedToInternet: !!evt.isConnectedToInternet });
          break;
        case 'ERROR':
          set({
            lastError: { op: evt.op, code: evt.code, message: evt.message },
          });
          break;
      }
    });

    await service.start();
  },

  // Store action (implementation)
  setIpAddress: async (name, ip) => {
    // Optional optimistic update
    set({
      ipAddress: ip,
      currentNetworkInterface: name,
      lastError: null,
    });

    try {
      await service.setIpAddress(name, ip);
      // On success, service emits IP_CHANGED, store updates consistently
    } catch (e) {
      set({
        lastError: { op: 'setIpAddress', code: e.code, message: e.message },
      });
    }
  },
}));