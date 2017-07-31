/* 	A space gas station where space vehicles come to fill up their tank 
	with nitrogen and quantum fluid and supply vehicles come to deposit either
	nitrogen or quantum fluid (and also fill up their own tanks). 
	The space station is represented as a monitor
	(a concurrent object) that both type of vehicles access. The vehicles
	use notify() and wait() to communicate with each other. 

Usage in Linux:
javac FuelSpaceStation.java
java FuelSpaceStation numVehicles

There is no fairness: only wait() and notify() are used as synchronization
methods and notify does not guarantee fairness: it randomly picks a thread
from the wait set.

*/

class SpaceStation{

private int N = 500, V = 4, Q = 500;
int nitrogen = N, quantumFluid = Q, dockingPlaces = V, numTrips;

public SpaceStation(int numTrips){
 this.numTrips = numTrips;
}

public int getNumTrips(){
 return numTrips;
}

private boolean not_ok_to_dock(int n, int q){
 return (dockingPlaces < 1) || (nitrogen < n) || (quantumFluid < q);
}

public void request_fuel(int myid, int n, int q, boolean supply){
 synchronized(this){
 System.out.println((supply == true? "\tSupply vehicle " : "Space vehicle ") + myid + " requests " + n + "l nitrogen and " + q + "l quantum fluid.\n " + 
 nitrogen + "l nitrogen and " + quantumFluid + "l quantum fluid left in station.\n  " + 
 dockingPlaces + " docking places left.\tnumTrips = " + numTrips + "\n");
/* if there are not enough docking places left, or there is not enough nitrogen or quantum fluid to 
   satisfy my requests, I'll wait to be notified and yield processor to other threads while waiting */
 while(not_ok_to_dock(n, q))
	try { this.wait(); } catch (InterruptedException e) { }
	dockingPlaces--;
 }

 get_fuel(myid, n, q, supply);

}


public void get_fuel(int myid, int n, int q, boolean supply){
 synchronized(this){
 System.out.println((supply == true? "\tSupply vehicle " : "Space vehicle ") + myid + " filling up tank w " + n + "l nitrogen and " + q 
 + "l quantum fluid.\n " + dockingPlaces + " docking places left\n");
 }

 synchronized(this){
 while(nitrogen > 0 &&  n > 0){ nitrogen--; n--;}
 while(quantumFluid > 0 &&  q > 0){ quantumFluid--; q--;}
 }
/* sleep to simulate time taken to fill up tank */
 try{Thread.sleep(1000 + (long)(Math.random() * 2000));} catch(InterruptedException e){}
 
 synchronized(this){
 dockingPlaces++;
 System.out.println((supply == true? "\tSupply vehicle " : "Space vehicle ") + myid + " finished filling up tank.\n " +
 nitrogen + "l nitrogen and " + quantumFluid + "l quantum fluid left in station.\n  " + 
 dockingPlaces + " docking places left\n");
 if(!supply) numTrips--;
 /* notify all threads that there is now one more docking place available */
 this.notifyAll();
 }
}

private boolean not_ok_to_deliver(int n, int q){
 if(n < q){
	return ((Q - quantumFluid) < q);
 } else {
	return ((N - nitrogen) < n);
 }
}

public synchronized void deliver_fuel(int myid, int n, int q)  {

 System.out.println("\tSupply vehicle " + myid + " waiting to deposit 400l" + ((q < n)? " nitrogen\n" : " quantum fluid\n"));
/* wait until there is enough capacity to deposit all gas */
 while(not_ok_to_deliver(n, q)){
	/* if all space vehicles are finished, return */
	if(numTrips < 1){return;} //TODO: kolla om det är ngn poäng att vänta högst 2 s i taget
	try { this.wait(2000); } catch (InterruptedException e) { }
 }
 
 System.out.println("\t\t" + "\tSupply vehicle " + myid + " deposits " + (n > 0? (n + "l n\n" ) : "") + (q > 0? (q + "l q\n" ) : ""));
/* deposit gas */
 nitrogen+=n;
 quantumFluid+=q;

 /* sleep to simulate time taken to deposit gas */
 try{ Thread.sleep(1000); } catch(InterruptedException e){}
 
 /* notify all threads that there is now more gas */
 this.notifyAll();
}

}


class SpaceVehicle extends Thread {
 private SpaceStation station;
 private int myid, n, q, capacity = 50, trips;

 public SpaceVehicle(SpaceStation station, int myid, int trips) {
 this.station = station;
 this.myid = myid;
 this.trips = trips;
 }

 public void run() {
 while(trips > 0){ 
	 /* sleep to simulate time it takes to get to gas station */
	 try{Thread.sleep((int) Math.random() * 6000);} catch(InterruptedException e){}
	 /* get random amount of nitrogen and quantum fluid, in the range of 0 and 50 */
	 n = (int) (Math.random() * 50);
	 q = (int) (Math.random() * 50);
	 /* fill up gas */
	 station.request_fuel(myid, n, q, false);
	 /* sleep to simulate time it takes to leave gas station */
	 try{Thread.sleep((int) Math.random() * 6000);} catch(InterruptedException e){}
	 trips--;
	 } 
 }

}

class SupplyVehicle extends Thread {

 private SpaceStation station;
 private int myid, n, q, type;

 public SupplyVehicle(SpaceStation station, int myid, int type) {
 this.station = station;
 this.myid = myid;
 this.type = type;
 }

 public void run() {
  
 while(station.getNumTrips() > 0){ 
	 /* sleep to simulate time it takes to get to gas station */
	 try{Thread.sleep((int) Math.random() * 10000);} catch(InterruptedException e){}
	 if(type == 1){
		 /* nitrogen supplier */
		 n = 400;
		 q = 0;
	 } else if(type == 0){
		 /* quantum fluid supplier */
		 n = 0;
		 q = 400;	 
	 }
	 /* deliver gas */
	 station.deliver_fuel(myid, n, q);
	 /* fill up own gas tank */
	 n = (int) (Math.random() * 50);
	 q = (int) (Math.random() * 50);
	 station.request_fuel(myid, n, q, true);
	 /* leave gas station */
	 try{Thread.sleep((int) Math.random() * 10000);} catch(InterruptedException e){}
	 }
 }

}


public class FuelSpaceStation {
 private static int maxVehicles = 20;
 public static void main(String[] args) {
  /* read command line argument, if any */
 int numVehicles = ((args.length > 0) ? Integer.parseInt(args[0]) : 10);
 if(numVehicles > maxVehicles) numVehicles = maxVehicles;
 int tripsPerVehicle = 5;
 SpaceStation station = new SpaceStation(numVehicles * tripsPerVehicle);
 /* create space vehicles */
 for(int i = 0; i < numVehicles; i++)
 	new SpaceVehicle(station, i, tripsPerVehicle).start();

 /* create 1/4 as many supplier vehicles as space vehicles,
 	ca 50/50 nitrogen/quantum fluid suppliers */
 for(int i = 0; i < (int)numVehicles/4; i++){
 	if(i%2 == 0){
		 new SupplyVehicle(station, i, 1).start();
 	} else {
		 new SupplyVehicle(station, i, 0).start();
 	}
 }
 }
}


