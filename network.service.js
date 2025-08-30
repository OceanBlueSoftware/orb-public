// network.service.js
// Minimal service wrapper — only fine-grained events, no snapshots.

export class NetworkService {
  constructor({ thunder } = {}) {
    this.thunder = thunder; // inject Thunder client if available
    this._listeners = new Set();
  }

  subscribe(fn) {
    this._listeners.add(fn);
    return () => this._listeners.delete(fn);
  }

  _emit(evt) {
    for (const fn of this._listeners) fn(evt);
  }

  async start() {
    // Example: subscribe to Thunder's "online" event
    // this.thunder.on('NetworkOnlineChanged', (online) => {
    //   this._emit({ type: 'STATUS', isConnectedToInternet: !!online });
    // });
  }

  async setIpAddress(name, ip) {
    try {
      // Call Thunder API here (pseudo)
      // await this.thunder.call('Network', 'setIpAddress', { interface: name, ipAddress: ip });

      // On success emit event
      this._emit({ type: 'IP_CHANGED', name, ip });
    } catch (err) {
      this._emit({
        type: 'ERROR',
        op: 'setIpAddress',
        code: err.code || 'E_SET_IP',
        message: String(err.message || err),
      });
      throw err;
    }
  }
}