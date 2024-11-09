import { FC, useCallback, useEffect, useState,ReactNode } from 'react';

import * as FeaturesApi from '../../api/features';

import { extractErrorMessage } from '../../utils';
import { Features } from '../../types';
import {ApplicationError, LoadingSpinner} from '../../components';

import { FeaturesContext } from '.';
import Dashboard from '../../project/Dashboard';
import AppRouting from '../../AppRouting'
interface FeaturesLoaderProps {
  children: ReactNode; // Define children prop here
}
const FeaturesLoader: FC<FeaturesLoaderProps> = (props) => {
  const [errorMessage, setErrorMessage] = useState<string>();
   
    return (
      <Dashboard />
    );
  

  if (errorMessage) {
    return (
      <ApplicationError message={errorMessage} />
    );
  }

  return (
    <LoadingSpinner height="100vh" />
  );

};

export default FeaturesLoader;
