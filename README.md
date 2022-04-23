![alt text](https://st3.depositphotos.com/1005049/31711/v/600/depositphotos_317110512-stock-illustration-garbage-man-at-work-lorry.jpg)

# Flash GC Simulator

This simulator enables to run the following garbage collection (GC) algorithms on an SSD memory layout:
1. Generational GC
2. Generational GCx (with static thresholds of your choice)
3. Generational GCx with Load-Balancing

We implemented the last 2 algorithms specified above based on the Simulator of Eyal Lotan and Dor Sura. The basic simulator has larger capabilities. Though we didn't remove all of them from the code, we might have broken them. So our recommandation will be to use our simulator only for the 3 algorithms listed above and only with the parametrs mentioned later in the file. If you wish to run other simulations, go to previouse version here: https://github.com/Eyallotan/GC_Simulatfully 
For all simulations you'll be able to record statistics such as number of erases and write amplification.

For full details about the theoretical model and the algorithms implemented in this simulator please read the [project report](https://github.com/MenuchaBen/Generational-GC-with-Load-Balancing/blob/main/Generational-GCx-with-Load-Balancingt.pdf). In the report you will find all the needed prerequisites, along with a full breakdown of each algorithm, experiments results and more. 

## Installation

Follow these steps:

```cmd
$ git clone https://github.com/MenuchaBen/Generational-GC-with-Load-Balancing.git
$ cd GC_Simulator
$ g++ *.cpp -o Simulator.exe
$ .\Simulator.exe <enter command line parameters>
```

## Usage

### Command Line Parameters

These are the parameters you must set for each simulation:
1. Number of physical blocks (T)
2. Number of logical blocks (U)
3. Pages per block (Z)
4. Page size (in bytes)
5. Number of pages (N) 
6. Window flag - should be "window_off"
7. Data distribution - should be "uniform"
8. GC algorithm - should be "generational"
9. BETA  

```
BETA parameter: 
this parameter is used differently by each algorithm. 
  1. Generational GC - does not use BETA, though you'll need to provide it either way.
  2. Generational GCx with static threshold - BETA is the threshold.
  3. Generational GCx with Load-Balancing - BETA is the interval size.
```  
You will be prompt to choose the number of generations and the algorithm type. For the algoritm type, you'll be presented with a menu to choose from. Enter the algorithm index in the menu you wish to run for your simulation. 
If you wish that the number of generations would be chosen for you based on the given parameters (T,U,Z) and according to the best results of the experiments done by us and by Eyal and Dor, enetr 0 when you are prompted to choose the number of generations for you simulation. 

For the algorithm Generational GC, you'll be able to see the logical load during the simulation on each generation.
For the other 2 algorithm, you'll see also the physical load.

### Examples

```cmd
$ .\Simulator.exe 96 81 256 4096 100000 window_off uniform generational 0
```

```cmd
Starting GC Simulator!
Physical Blocks:        96
Logical Blocks:         81
Pages/Block:            256
Page Size:              4096
Alpha:                  0.84375
Beta:                   0
Over Provisioning:      0.185185
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for auto selection):
3
Enter the number of generational algorithm you wish to run:
1 - Generational GC, BETA is not used
2 - Generational GCx with static bound, BETA is the static bound
3 - Generational GCx with Load-Balancing - BETA is interval size
1
generational algorithm type set to 1
Number of generations set to 3 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Writes per generation:
Generation 0 logical writes:    33.375%
Generation 1 logical writes:    22.509%
Generation 2 logical writes:    44.116%
Number of erases: 1291. Write Amplification: 3.30459
```

```cmd
$ .\Simulator.exe 96 81 256 4096 100000 window_off uniform generational 0
```
```
Starting GC Simulator!
Physical Blocks:        96
Logical Blocks:         81
Pages/Block:            256
Page Size:              4096
Alpha:                  0.84375
Beta:                   0
Over Provisioning:      0.185185
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for auto selection):
0
Enter the number of generational algorithm you wish to run:
1 - Generational GC, BETA is not used
2 - Generational GCx with static bound, BETA is the static bound
3 - Generational GCx with Load-Balancing - BETA is interval size
1
generational algorithm type set to 1
Using Overloading factor heuristic to select number of generations...
Number of generations set to 5 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Writes per generation:
Generation 0 logical writes:    21.578%
Generation 1 logical writes:    17.123%
Generation 2 logical writes:    13.361%
Generation 3 logical writes:    10.472%
Generation 4 logical writes:    37.466%
Number of erases: 1316. Write Amplification: 3.36603
```

```cmd
$ .\Simulator.exe 96 81 256 4096 100000 window_off uniform generational 4147
```
```
Starting GC Simulator!
Physical Blocks:        96
Logical Blocks:         81
Pages/Block:            256
Page Size:              4096
Alpha:                  0.84375
Beta:                   4147
Over Provisioning:      0.185185
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for auto selection):
2
Enter the number of generational algorithm you wish to run:
1 - Generational GC, BETA is not used
2 - Generational GCx with static bound, BETA is the static bound
3 - Generational GCx with Load-Balancing - BETA is interval size
2
generational algorithm type set to 2
Number of generations set to 2 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Writes per generation:
Generation 0 logical writes:    21.368%
Generation 0 physical writes:   36.8374%
Generation 1 logical writes:    78.632%
Generation 1 physical writes:   63.1626%
Number of erases: 993. Write Amplification: 2.54174
```
```cmd
$ .\Simulator.exe 96 81 256 4096 100000 window_off uniform generational 4147
```
```
Starting GC Simulator!
Physical Blocks:        96
Logical Blocks:         81
Pages/Block:            256
Page Size:              4096
Alpha:                  0.84375
Beta:                   4147
Over Provisioning:      0.185185
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for auto selection):
0
Enter the number of generational algorithm you wish to run:
1 - Generational GC, BETA is not used
2 - Generational GCx with static bound, BETA is the static bound
3 - Generational GCx with Load-Balancing - BETA is interval size
2
generational algorithm type set to 2
Number of generations set to 8 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Writes per generation:
Generation 0 logical writes:    21.339%
Generation 0 physical writes:   40.9485%
Generation 1 logical writes:    16.939%
Generation 1 physical writes:   12.6191%
Generation 2 logical writes:    13.335%
Generation 2 physical writes:   9.88226%
Generation 3 logical writes:    10.714%
Generation 3 physical writes:   7.99029%
Generation 4 logical writes:    8.441%
Generation 4 physical writes:   6.21141%
Generation 5 logical writes:    6.638%
Generation 5 physical writes:   4.96375%
Generation 6 logical writes:    5.112%
Generation 6 physical writes:   3.80656%
Generation 7 logical writes:    17.482%
Generation 7 physical writes:   13.5782%
Number of erases: 643. Write Amplification: 1.63586
```
```cmd
$ .\Simulator.exe 96 81 256 4096 100000 window_off uniform generational 4147
```
```
Starting GC Simulator!
Physical Blocks:        96
Logical Blocks:         81
Pages/Block:            256
Page Size:              4096
Alpha:                  0.84375
Beta:                   4147
Over Provisioning:      0.185185
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for auto selection):
0
Enter the number of generational algorithm you wish to run:
1 - Generational GC, BETA is not used
2 - Generational GCx with static bound, BETA is the static bound
3 - Generational GCx with Load-Balancing - BETA is interval size
3
generational algorithm type set to 3
Number of generations set to 8 generations.

Enter gen population:
0.1 0.1 0.1 0.1 0.15 0.15 0.15 0.15
Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Writes per generation:
Generation 0 logical writes:    9.959%
Generation 0 physical writes:   26.9267%
Generation 1 logical writes:    9.921%
Generation 1 physical writes:   9.45%
Generation 2 logical writes:    9.914%
Generation 2 physical writes:   8.26267%
Generation 3 logical writes:    9.555%
Generation 3 physical writes:   7.662%
Generation 4 logical writes:    14.611%
Generation 4 physical writes:   11.6493%
Generation 5 logical writes:    15.6%
Generation 5 physical writes:   12.384%
Generation 6 logical writes:    14.573%
Generation 6 physical writes:   11.5827%
Generation 7 logical writes:    15.395%
Generation 7 physical writes:   12.0827%
Number of erases: 589. Write Amplification: 1.50711
```

## Contributing

Pull requests are welcomed. 

## License
All rights are reserved to Menucha Benisti and Rotem Elias.
The Simulator FTL interface is based on the work of Eyal Lotan, Dor Sura and Alex Yucovich.

