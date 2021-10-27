-- new SELF_TYPE creates an object with the same dynamic type as self,
-- which affects initialization of the new object's attributes.


class Base inherits IO
{
  baseAttr : Int <- {
  out_string("Calling Base::report from baseAttr.init\n");
  report(1);
  out_string("Leaving Base::report from baseAttr.init\n");
   1;};

  report( value : Int ) : SELF_TYPE
  {
    {
    out_string("base:");
      out_int( value );
      out_string( "\n" );
      out_string("leaving base\n");
      self;
    }
  };

  duplicate() : Base
  {
    new SELF_TYPE
  };
};


class Derived inherits Base
{
  derivedAttr : Int <- {
  out_string("Calling Derived::report from derivedAttr.init\n");
  out_int(derivedAttr);
  report(2);
  out_string("Leaving Derived::report from derivedAttr.init\n");
  2;};

  report( value : Int ) : SELF_TYPE
  { 
    {
    	out_string("derived:");
      out_string("old: ");
      out_int(baseAttr);
      out_int(derivedAttr);
      out_string(".  new: ");
      derivedAttr <- value;
      self@Base.report( derivedAttr );
      -- out_string("leaving derived\n");
    }
  };
};


class Main 
{
  main() : Object
  {
  (new IO).out_int(1145141)
  --new Derived
    --(new Derived).report ( 5 ).duplicate().report ( 29 )
  };
};
