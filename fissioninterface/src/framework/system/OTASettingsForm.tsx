import React, { FC, useState, useEffect } from 'react';
import { ValidateFieldsError } from 'async-validator';
import SystemService from "../../api/system";
import { Button, Checkbox } from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';

import { OTASettings } from '../../types';
import { BlockFormControlLabel, ButtonRow, FormLoader, SectionContent, ValidatedPasswordField, ValidatedTextField } from '../../components';
import { validate, OTA_SETTINGS_VALIDATOR } from '../../validators';
import { numberValue, updateValue } from '../../utils';

const OTASettingsForm: FC = () => {
  const [fieldErrors, setFieldErrors] = useState<ValidateFieldsError>();
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>();
  const [saving, setSaving] = useState<boolean>(false);
  const [data, setData] = useState<OTASettings | null>(null);

  const fetchOTAData = async () => {
    setLoading(true);
    try {
      const response = await SystemService.readOTASettings();
      setData(response);
    } catch (err) {
      setError('Failed to load OTA settings.');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  const updateFormValue = updateValue(setData);

  const validateAndSubmit = async () => {
    try {
      if (!data) return;
      setFieldErrors(undefined);
      setSaving(true);
      await validate(OTA_SETTINGS_VALIDATOR, data);
      await SystemService.updateOTASettings(data);
    } catch (errors: any) {
      setFieldErrors(errors);
      console.error(errors);
    } finally {
      setSaving(false);
    }
  };

  useEffect(() => {
    fetchOTAData();
  }, []);

  const content = () => {
    if (loading) {
      return <FormLoader onRetry={fetchOTAData} errorMessage={error} />;
    }

    return (
      <>
        <BlockFormControlLabel
          control={
            <Checkbox
              name="enabled"
              checked={data?.enabled}
              onChange={updateFormValue}
            />
          }
          label="Enable OTA Updates?"
        />
        <ValidatedTextField
          fieldErrors={fieldErrors}
          name="port"
          label="Port"
          fullWidth
          variant="outlined"
          value={numberValue(data?.port ?? 0)}
          type="number"
          onChange={updateFormValue}
          margin="normal"
        />
        <ValidatedPasswordField
          fieldErrors={fieldErrors}
          name="password"
          label="Password"
          fullWidth
          variant="outlined"
          value={data?.password}
          onChange={updateFormValue}
          margin="normal"
        />
        <ButtonRow mt={1}>
          <Button startIcon={<SaveIcon />} disabled={saving} variant="contained" color="primary" onClick={validateAndSubmit}>
            Save
          </Button>
        </ButtonRow>
      </>
    );
  };

  return (
    <SectionContent title='OTA Settings' titleGutter>
      {content()}
    </SectionContent>
  );
};

export default OTASettingsForm;
