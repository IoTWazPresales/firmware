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

  const isActive = routeMatches(to, pathname);

  return (
    <ListItem disablePadding selected={isActive}>
      <ListItemButton 
        component={Link} 
        to={to} 
        onClick={handleClick} 
        disabled={disabled}
        sx={{
          ...(isActive && {
            backgroundColor: 'rgba(0, 212, 255, 0.15)',
            borderLeft: '3px solid #00D4FF',
            boxShadow: 'inset 0 0 10px rgba(0, 212, 255, 0.2)',
            '& .MuiListItemIcon-root': {
              color: '#00D4FF',
            },
            '& .MuiListItemText-primary': {
              color: '#00D4FF',
              fontWeight: 600,
              textShadow: '0 0 8px rgba(0, 212, 255, 0.4)',
            },
          }),
          '&:hover': {
            backgroundColor: isActive ? 'rgba(0, 212, 255, 0.2)' : 'rgba(0, 212, 255, 0.08)',
            boxShadow: isActive 
              ? 'inset 0 0 15px rgba(0, 212, 255, 0.3)' 
              : 'inset 0 0 8px rgba(0, 212, 255, 0.15)',
            '& .MuiListItemIcon-root': {
              color: isActive ? '#00FFFF' : 'rgba(0, 212, 255, 0.7)',
            },
          },
          transition: 'all 0.3s ease',
        }}
      >
        <ListItemIcon>
          <Icon />
        </ListItemIcon>
        <ListItemText>{label}</ListItemText>
      </ListItemButton>
    </ListItem>
  );
};

export default LayoutMenuItem;
