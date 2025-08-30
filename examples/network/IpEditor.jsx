import React, { useState } from 'react';
import { useNetwork } from './useNetwork.js';

export function IpEditor() {
  const { currentNetworkInterface, ipAddress, setIpAddress, lastError } = useNetwork();
  const [newIp, setNewIp] = useState(ipAddress || '');

  return (
    <div>
      <div>Current IF: {currentNetworkInterface || '—'}</div>
      <div>Current IP: {ipAddress || '—'}</div>

      <input
        type="text"
        value={newIp}
        onChange={(e) => setNewIp(e.target.value)}
        placeholder="Enter new IP"
      />
      <button onClick={() => setIpAddress(currentNetworkInterface, newIp)}>
        Save
      </button>

      {lastError && (
        <div style={{ color: 'red' }}>Error: {lastError.message}</div>
      )}
    </div>
  );
}

