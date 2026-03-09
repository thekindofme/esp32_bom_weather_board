import React, { useState, useEffect } from 'react';
import { useLayoutStore } from '../store/layoutStore';
import * as api from '../api/client';

export const FlashModal: React.FC = () => {
  const { flashModalOpen, setFlashModalOpen, layout } = useLayoutStore();
  const [ports, setPorts] = useState<string[]>([]);
  const [selectedPort, setSelectedPort] = useState('');
  const [flashing, setFlashing] = useState(false);
  const [output, setOutput] = useState('');
  const [codePreview, setCodePreview] = useState('');
  const [tab, setTab] = useState<'flash' | 'code'>('flash');
  const [loadingPorts, setLoadingPorts] = useState(false);

  useEffect(() => {
    if (flashModalOpen) {
      refreshPorts();
    }
  }, [flashModalOpen]);

  const applyPorts = (nextPorts: string[]) => {
    setPorts(nextPorts);
    setSelectedPort(current => {
      if (nextPorts.length === 0) return '';
      if (current && nextPorts.includes(current)) return current;
      return nextPorts[0];
    });
  };

  const refreshPorts = async () => {
    setLoadingPorts(true);
    try {
      const p = await api.getDevicePorts();
      applyPorts(p);
    } catch {
      applyPorts([]);
    }
    setLoadingPorts(false);
  };

  const handleFlash = async () => {
    setFlashing(true);
    setOutput('Generating C++ code and compiling...\n');
    try {
      const result = await api.flashLayout(layout, selectedPort);
      setOutput(result.output);
    } catch (err) {
      if (err instanceof api.ApiError) {
        setOutput(err.details ?? err.message);
      } else {
        setOutput(`Flash failed: ${String(err)}`);
      }
    }
    setFlashing(false);
  };

  const handlePreviewCode = async () => {
    try {
      const result = await api.generateCode(layout);
      setCodePreview(result.code);
    } catch (err) {
      if (err instanceof api.ApiError) {
        setCodePreview(`// ${err.details ?? err.message}`);
      } else {
        setCodePreview(`// Error generating code: ${String(err)}`);
      }
    }
    setTab('code');
  };

  if (!flashModalOpen) return null;

  return (
    <div className="modal-overlay" onClick={() => !flashing && setFlashModalOpen(false)}>
      <div className="modal-content" onClick={e => e.stopPropagation()}>
        <div className="modal-header">
          <h2>Flash to ESP32</h2>
          <button className="modal-close" onClick={() => !flashing && setFlashModalOpen(false)}>x</button>
        </div>

        <div className="modal-tabs">
          <button className={`modal-tab ${tab === 'flash' ? 'active' : ''}`} onClick={() => setTab('flash')}>Flash</button>
          <button className={`modal-tab ${tab === 'code' ? 'active' : ''}`} onClick={handlePreviewCode}>C++ Preview</button>
        </div>

        {tab === 'flash' ? (
          <div className="modal-body">
            <div className="flash-port-section">
              <label>Serial Port</label>
              <div className="port-row">
                <select
                  value={selectedPort}
                  onChange={e => setSelectedPort(e.target.value)}
                  disabled={flashing}
                >
                  {ports.length === 0 && <option value="">No ports detected</option>}
                  {ports.map(p => <option key={p} value={p}>{p}</option>)}
                </select>
                <button className="btn-secondary btn-small" onClick={refreshPorts} disabled={loadingPorts}>
                  {loadingPorts ? '...' : 'Refresh'}
                </button>
              </div>
            </div>

            {output && (
              <div className="flash-output">
                <pre>{output}</pre>
              </div>
            )}

            <div className="modal-actions">
              <button
                className="btn-primary flash-action-btn"
                onClick={handleFlash}
                disabled={flashing || !selectedPort}
              >
                {flashing ? (
                  <><span className="spinner-tiny" /> Flashing...</>
                ) : (
                  <>
                    <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M8 1L3 8h4l-1 5 5-7H7l1-5z" fill="currentColor"/></svg>
                    Flash Layout
                  </>
                )}
              </button>
            </div>
          </div>
        ) : (
          <div className="modal-body">
            <div className="code-preview">
              <pre><code>{codePreview || '// Click to generate C++ code preview...'}</code></pre>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};
