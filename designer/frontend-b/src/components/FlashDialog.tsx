import React, { useState, useEffect } from 'react';
import { useLayoutStore } from '../store/layoutStore';
import * as api from '../api/client';

export const FlashDialog: React.FC = () => {
  const { flashDialogOpen, setFlashDialogOpen, layout } = useLayoutStore();
  const [ports, setPorts] = useState<string[]>([]);
  const [port, setPort] = useState('');
  const [flashing, setFlashing] = useState(false);
  const [output, setOutput] = useState('');
  const [tab, setTab] = useState<'flash' | 'code'>('flash');
  const [code, setCode] = useState('');

  useEffect(() => {
    if (flashDialogOpen) {
      refreshPorts();
    }
  }, [flashDialogOpen]);

  const applyPorts = (nextPorts: string[]) => {
    setPorts(nextPorts);
    setPort(current => {
      if (nextPorts.length === 0) return '';
      if (current && nextPorts.includes(current)) return current;
      return nextPorts[0];
    });
  };

  const refreshPorts = async () => {
    try {
      applyPorts(await api.getDevicePorts());
    } catch {
      applyPorts([]);
    }
  };

  const handleFlash = async () => {
    setFlashing(true);
    setOutput('Compiling and flashing...\n');
    try {
      const r = await api.flashLayout(layout, port);
      setOutput(r.output);
    } catch (e) {
      if (e instanceof api.ApiError) {
        setOutput(e.details ?? e.message);
      } else {
        setOutput(`Error: ${String(e)}`);
      }
    }
    setFlashing(false);
  };

  const handleCode = async () => {
    try {
      setCode((await api.generateCode(layout)).code);
    } catch (e) {
      if (e instanceof api.ApiError) {
        setCode(`// ${e.details ?? e.message}`);
      } else {
        setCode(`// ${String(e)}`);
      }
    }
    setTab('code');
  };

  if (!flashDialogOpen) return null;

  return (
    <div className="dialog-backdrop" onClick={() => !flashing && setFlashDialogOpen(false)}>
      <div className="dialog" onClick={e => e.stopPropagation()}>
        <div className="dialog-header">
          <h2>Flash to Device</h2>
          <button className="dialog-close" onClick={() => !flashing && setFlashDialogOpen(false)}>&times;</button>
        </div>
        <div className="dialog-tabs">
          <button className={`dialog-tab ${tab === 'flash' ? 'active' : ''}`} onClick={() => setTab('flash')}>Flash</button>
          <button className={`dialog-tab ${tab === 'code' ? 'active' : ''}`} onClick={handleCode}>C++ Preview</button>
        </div>
        <div className="dialog-body">
          {tab === 'flash' ? (
            <>
              <div className="dialog-field">
                <label>Serial Port</label>
                <div className="dialog-port-row">
                  <select value={port} onChange={e => setPort(e.target.value)} disabled={flashing}>
                    {ports.length === 0 && <option value="">None detected</option>}
                    {ports.map(p => <option key={p} value={p}>{p}</option>)}
                  </select>
                  <button className="btn-outline btn-sm" onClick={refreshPorts}>Refresh</button>
                </div>
              </div>
              {output && <pre className="dialog-output">{output}</pre>}
              <button className="btn-flash" onClick={handleFlash} disabled={flashing || !port}>
                {flashing ? 'Flashing...' : 'Flash Layout'}
              </button>
            </>
          ) : (
            <pre className="dialog-code">{code || '// Generate code...'}</pre>
          )}
        </div>
      </div>
    </div>
  );
};
