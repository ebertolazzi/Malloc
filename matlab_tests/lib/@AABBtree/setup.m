% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
function setup( self, NOBJ, long, vtol )
  % bound population count
  if NOBJ <= 0
    error('NOBJ must be > 0, found %g', NOBJ );
  end
  % bound "long" tolerance
  if long < 0 || long > 1
    error('long must be in (0,1) found %g', long );
  end
  % bound "vtol" tolerance
  if vtol < 0 || vtol > 1
    error('vtol must be in (0,1) found %g', vtol );
  end
  self.NOBJ = NOBJ;
  self.long = long;
  self.vtol = vtol;
end
