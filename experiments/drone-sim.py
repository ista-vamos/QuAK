from math import sqrt
from numpy.random import normal as normal_distr
from gen_smooth import get_symbol
import argparse

MAX_FORCE = 10

def p_dist(P1, P2):
    x = P2[0] - P1[0]
    y = P2[1] - P1[1]
    return sqrt(x*x + y*y)



class Drone:
    """
    A similified model of drone. We assume its mass is 1,
    so that the acceleration is equal to the force
    made by thurst of propellers.

    For even more simplicity, we assume the drone is a point.
    """

    def __init__(self, pos, target):
        # velocity
        self.v = (0, 0)
        self.pos = pos
        self.target = target
        self.energy_consumption = 0
        self.commands = []


    def apply_force(self, force, dt):
        """
        Command the drone to apply the given force,
        resulting in acceleration acc. to the force.
        """
        dx = force[0]*dt
        dy = force[1]*dt
        x, y = self.pos
        x, y = x + dx, y + dy
        self.pos = (x, y)

        self.commands.append(force)
        self.energy_consumption += sqrt(force[0]**2 + force[1]**2)


    def step(self, arena, dt):
        """
        Do a next step towards the target.
        `dt` is the time difference to the next state
        """
        pass





# NOT USED RIGHT NOW
class Arena:
    def __init__(self):
        pass
       #self.obstacle = [
       #    (x, y) for x in range(30, 80 + 1) for y in range (30, 60 + 1)
       #]


class Simulation:
    def __init__(self, arena, drone):
        self.arena = arena
        self.drone = drone
        self.dt = 0.1

    def run(self):
        drone = self.drone
        arena = self.arena
        dt = self.dt
        target = drone.target

        trajectory = [drone.pos]
        while p_dist(drone.pos, target) > 0.01:
            drone.step(arena, dt)

            trajectory.append(drone.pos)

        # add the target point for as the final destination
        trajectory.append(target)
        return trajectory

class DroneRand(Drone):
    def __init__(self, pos, target, sd=1):
        super().__init__(pos, target)
        self.sd = sd

    def step(self, arena, dt):
        target = self.target
        pos = self.pos

        vx = (target[0] - pos[0])
        vy = (target[1] - pos[1])
        s = sqrt(vx*vx + vy*vy)
        # normalize to get just the direction
        vx = vx/s
        vy = vy/s

        # pick at random with a preference towards the target
        sd = self.sd
        sd2 = sd*sd
        vx = normal_distr(MAX_FORCE*vx, sd2)
        vy = normal_distr(MAX_FORCE*vy, sd2)
        if vx > MAX_FORCE: vx = MAX_FORCE
        if vy > MAX_FORCE: vy = MAX_FORCE
        if vx < -MAX_FORCE: vx = -MAX_FORCE
        if vy < -MAX_FORCE: vy = -MAX_FORCE

        self.apply_force((vx, vy), dt)


class DroneFollow(Drone):
    def __init__(self, pos, target, traj, dt=None):
        super().__init__(pos, target)
        self.traj = traj
        self.idx = 0
        self.dt = dt

    def step(self, arena, dt):
        self.idx += 1
        idx = self.idx

        if idx < len(self.traj):
            new_pos = self.traj[idx]
        else:
            new_pos = self.target

        pos = self.pos
        dx = new_pos[0] - pos[0]
        dy = new_pos[1] - pos[1]
        self.apply_force((dx, dy), self.dt or dt)



def dump_data(drone, trajectory, suff=""):
    with open(f"traj{suff}.txt", "w") as out:
        for p in trajectory:
            print(f"{p[0]},{p[1]}", file=out)

    with open(f"forces{suff}.txt", "w") as out:
        for p in drone.commands:
            x, y = int(p[0]), int(p[1])
            print(get_symbol(x,y), file=out)

    #print("Energy consumption: ", drone.energy_consumption)
    print("Avg energy consumption per step: ", drone.energy_consumption / len(trajectory))


# Inspired by code from https://medium.com/@jaems33/understanding-robot-motion-path-smoothing-5970c8363bc4
def smooth(path, weight_data=0.5, weight_smooth=0.1, tolerance=0.000001):
#def smooth(path, weight_data=0.01, weight_smooth=0.95, tolerance=0.001):
    new = path[:]
    change = tolerance

    while change >= tolerance:
        change = 0.0
        for i in range(1, len(new) - 1):
                old_x = path[i][0]
                old_y = path[i][1]

                new_x, x_prev, x_next = new[i][0], new[i - 1][0], new[i + 1][0]
                new_y, y_prev, y_next = new[i][1], new[i - 1][1], new[i + 1][1]
                new_x_saved = new_x
                new_y_saved = new_y

                new_x += weight_data * (old_x - new_x) + weight_smooth * (x_next + x_prev - (2 * new_x))
                new_y += weight_data * (old_y - new_y) + weight_smooth * (y_next + y_prev - (2 * new_y))

                new[i] = (new_x, new_y)

                change += abs(new_x - new_x_saved)
                change += abs(new_y - new_y_saved)
    return new



if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--sd", help="Standard deviation", action='store', type=int, default=1)
    args = parser.parse_args()

    arena = Arena()

    TARGET=(1000, 1000)
    print("## Drone with random controller")
    drone = DroneRand(pos=(0,0), target=TARGET, sd=args.sd)
    traj = Simulation(arena, drone).run()
    dump_data(drone, traj)

    #print("(computing the smooth trajectory)", end=" ")
    smooth_traj = smooth(traj)
    #print("done!")

    print("\n## Drone follow smoothed traj")
    drone = DroneFollow(pos=(0,0), target=TARGET, traj=smooth_traj)
    Simulation(arena, drone).run()
    dump_data(drone, traj, suff="-smooth")

