% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
function plot( self, varargin )
  if nargin > 0
    plot( [self.Pa(1), self.Pb(1)], [self.Pa(2), self.Pb(2)], varargin{:} );
  else
    plot( ...
      [self.Pa(1), self.Pb(1)], [self.Pa(2), self.Pb(2)], ...
      'o-b', 'LineWidth', 2, 'MarkerSize', 10, 'MarkerFaceColor', 'blue' ...
    );
  end
end
