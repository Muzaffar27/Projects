Compiling for questions:
----------------------------

Run 'make' in the respective question directory to compile the code

Run 'make clean' to clean.

command to run to exe after typing make: 

for ex:

./main 3 3200 tasks.txt

I tried several times, it did not deadlock, I hope it does not.
It should end after ending all tasks.
If it's stuck....CURSE YOU DEADLOCK. pls try run it again to mark it.
(really need those marks)


Questions:
How does the value of S affect turnaround time and response time? 
Is the difference in turnaround time and response time what you expected to see as S and the number of CPUs change? Why or why not?
How does adjusting the S value in the system affect the turnaround time or response time for long-running and I/O tasks specifically? Does it appear to be highly correlated?

Answers:
1. Effect of S on Turnaround Time and Response Time:

Smaller S values generally lead to lower response times as jobs are quickly moved to higher-priority queues. However, extremely small S values may increase turnaround time due to frequent queue changes.

2. Difference in Turnaround Time and Response Time with Changing S and CPUs:

As S decreases or the number of CPUs increases, response time tends to decrease, but the impact on turnaround time may vary. The difference in turnaround and response time is influenced by task characteristics and scheduling policies

3. Adjusting S and its Impact on Long-Running and I/O Tasks:

Lowering S can improve response time for I/O tasks, as they may be quickly promoted to higher-priority queues. For long-running tasks, S adjustments may have a moderate impact on turnaround time, but the correlation is not always high due to other scheduling factors.

