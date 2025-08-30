// useNetwork.js
import { useNetworkStore } from './network.store.js';

export function useNetwork() {
  // Selectors for state
  const ipAddress = useNetworkStore((s) => s.ipAddress);
  const currentNetworkInterface = useNetworkStore((s) => s.currentNetworkInterface);
  const isConnectedToInternet = useNetworkStore((s) => s.isConnectedToInternet);
  const lastError = useNetworkStore((s) => s.lastError);

  // Expose store actions
  const setIpAddress = useNetworkStore((s) => s.setIpAddress);

  return {
    ipAddress,
    currentNetworkInterface,
    isConnectedToInternet,
    lastError,
    setIpAddress, // pass-through action
  };
}

