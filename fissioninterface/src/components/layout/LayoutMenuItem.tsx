import React, { FC } from "react";
import { Link, useLocation, useNavigate } from "react-router-dom";
import {useState} from "react"
import { ListItem, ListItemButton, ListItemIcon, ListItemText, SvgIconProps } from "@mui/material";

import { routeMatches } from "../../utils/route";

interface LayoutMenuItemProps {
  icon: React.ComponentType<SvgIconProps>;
  label: string;
  to: string;
  disabled?: boolean;
}

const LayoutMenuItem: FC<LayoutMenuItemProps> = ({ icon: Icon, label, to, disabled }) => {
  const { pathname } = useLocation();
  const navigate = useNavigate();



  const [key, setKey] = useState(0);

  const handleClick = () => {
     console.log("Navigating to", to);
  };

  return (
    <ListItem disablePadding selected={routeMatches(to, pathname)}>
      <ListItemButton component={Link} to={to} onClick={handleClick} disabled={disabled}>
        <ListItemIcon>
          <Icon />
        </ListItemIcon>
        <ListItemText>{label}</ListItemText>
      </ListItemButton>
    </ListItem>
  );
};

export default LayoutMenuItem;
