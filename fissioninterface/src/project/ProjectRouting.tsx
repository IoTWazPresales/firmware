import { FC } from 'react';
import { Navigate, Routes, Route } from 'react-router-dom';

import GrowBox from './GrowBox';

const ProjectRouting: FC = () => {
  return (
    <Routes>
      {
        // Add the default route for your project below
      }
      <Route path="/*" element={<Navigate to="growbox" />} />
      {
        // Add your project page routes below.
      }
      <Route path="growbox/*" element={<GrowBox />} />
    </Routes>
  );
};

export default ProjectRouting;
