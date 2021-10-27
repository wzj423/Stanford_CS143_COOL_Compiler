(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

class StackCommand {
	data: String;
	next: StackCommand;
	init(d: String,nxtCmd:StackCommand):Object {
		{
		data<-d;
		next<-nxtCmd;
		}
	};
	getNext():StackCommand {
		next
	};
	setNext(nxtCmd:StackCommand):Object{
		next<-nxtCmd
	};
	getStr(): String {
		{
		(new IO).out_string("Undefined Behaivour from parent class\n") ;
		data;
		}
	};
};
class Main inherits IO {

   main() : Object {
		{
	(new StackCommand).getStr();
      out_string("Nothing implemented\n");
		}
   };

};
