all:	part1.out part2.out part3.out part4.out iobound cpubound

part1.out:	part1.c
				gcc -o part1.out part1.c
part2.out:	part2.c
				gcc -o part2.out part2.c
part3.out:	part3.c
				gcc -o part3.out part3.c
part4.out:	part4.c
				gcc -o part4.out part4.c

iobound:	iobound.c
					gcc -o iobound iobound.c
cpubound: cpubound.c
					gcc -o cpubound cpubound.c
clean:
						rm part1.out part2.out part3.out part4.out iobound cpubound
