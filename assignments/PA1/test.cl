Class Main {
    data:P;
   main(): Object{
   {
     data<- new C;
     data.foo();
   }
   };
};

Class P{
 foo():Object{
    (new IO).out_string("P")
 };
};

Class C inherits P {
 foo():Object{
    (new IO).out_string("C")
 };
};
