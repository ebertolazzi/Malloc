classdef TestClass < matlab.mixin.Copyable

  properties (SetAccess = private, Hidden = true)
    objectHandle;
  end
  methods(Access = protected)
    % Override copyElement method:
    function obj = copyElement( self )
      obj = copyElement@matlab.mixin.Copyable(self);
      obj.objectHandle = TestClassMexWrapper( 'copy', self.objectHandle );
    end
  end
  methods
    function self = TestClass( varargin )
      self.objectHandle = TestClassMexWrapper( 'new', varargin{:} );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function str = is_type( ~ )
      str = 'TestClass';
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function handle = handle( self )
      handle = self.objectHandle;
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function delete(self)
      TestClassMexWrapper( 'delete', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function res = a( self )
      res = TestClassMexWrapper( 'a', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function res = b( self )
      res = TestClassMexWrapper( 'b', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function res = c( self )
      res = TestClassMexWrapper( 'c', self.objectHandle );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_a( self, a )
      TestClassMexWrapper( 'set_a', self.objectHandle, a );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_b( self, b )
      TestClassMexWrapper( 'set_b', self.objectHandle, b );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function set_c( self, c )
      TestClassMexWrapper( 'set_c', self.objectHandle, c );
    end
    % - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    function info( self )
      TestClassMexWrapper( 'info', self.objectHandle );
    end
  end
end
