export type SensorValues = Record<string, number | string | null | undefined>;

export type SensorValueKind = 'numeric' | 'text';

export interface SensorCapabilityMeta {
    id: string;
    label: string;
    unit: string;
    kind: SensorValueKind;
}

export interface SensorResponse {
    values: SensorValues;
    meta: SensorCapabilityMeta[];
    /**
     * For backward compatibility we retain the flattened properties
     * until the UI migrates fully to `values`.
     */
    [legacyKey: string]: any;
}