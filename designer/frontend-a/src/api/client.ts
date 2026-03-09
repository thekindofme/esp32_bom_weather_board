const API = '/api';

export interface LayoutElement {
  id: string;
  type: string;
  x: number;
  y: number;
  width: number;
  height: number;
  zIndex: number;
  properties: Record<string, unknown>;
}

export interface Layout {
  id: string;
  name: string;
  orientation: 'portrait' | 'landscape';
  width: number;
  height: number;
  rotation: number;
  backgroundColor: string;
  elements: LayoutElement[];
  createdAt?: string;
  updatedAt?: string;
}

export interface LayoutMeta {
  id: string;
  name: string;
  orientation: string;
  createdAt: string;
  updatedAt: string;
}

export class ApiError extends Error {
  status: number;
  details?: string;

  constructor(status: number, message: string, details?: string) {
    super(message);
    this.name = 'ApiError';
    this.status = status;
    this.details = details;
  }
}

async function request<T>(url: string, options?: RequestInit): Promise<T> {
  const res = await fetch(url, {
    ...options,
    headers: {
      'Content-Type': 'application/json',
      ...options?.headers,
    },
  });
  if (!res.ok) {
    const text = await res.text();
    let message = `API error ${res.status}`;
    let details = text;

    if (text) {
      try {
        const parsed = JSON.parse(text) as Record<string, unknown>;
        if (typeof parsed.error === 'string' && parsed.error) {
          message = parsed.error;
        } else if (typeof parsed.message === 'string' && parsed.message) {
          message = parsed.message;
        } else {
          message = text;
        }

        if (typeof parsed.output === 'string' && parsed.output) {
          details = parsed.output;
        } else if (typeof parsed.error === 'string' && parsed.error) {
          details = parsed.error;
        }
      } catch {
        message = text;
      }
    }

    throw new ApiError(res.status, message, details || undefined);
  }
  if (res.status === 204) return undefined as T;
  return res.json();
}

export async function listLayouts(): Promise<Layout[]> {
  return request<Layout[]>(`${API}/layouts`);
}

export async function getLayout(id: string): Promise<Layout> {
  return request<Layout>(`${API}/layouts/${id}`);
}

export async function createLayout(layout: Omit<Layout, 'id' | 'createdAt' | 'updatedAt'>): Promise<Layout> {
  return request<Layout>(`${API}/layouts`, {
    method: 'POST',
    body: JSON.stringify(layout),
  });
}

export async function updateLayout(id: string, layout: Partial<Layout>): Promise<Layout> {
  return request<Layout>(`${API}/layouts/${id}`, {
    method: 'PUT',
    body: JSON.stringify(layout),
  });
}

export async function deleteLayout(id: string): Promise<void> {
  return request<void>(`${API}/layouts/${id}`, { method: 'DELETE' });
}

export async function flashLayout(layout: Layout, port: string): Promise<{ output: string; success: boolean }> {
  return request<{ output: string; success: boolean }>(`${API}/flash`, {
    method: 'POST',
    body: JSON.stringify({ port, layout }),
  });
}

export async function getDevicePorts(): Promise<string[]> {
  return request<string[]>(`${API}/device/ports`);
}

export async function generateCode(layout: Layout): Promise<{ code: string }> {
  return request<{ code: string }>(`${API}/codegen`, {
    method: 'POST',
    body: JSON.stringify(layout),
  });
}
