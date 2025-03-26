
import subprocess

def main():
    command = './solver_main'
    toplay = 'w'
    for i in range(16, 40):
        print("solving boardsize: 1x"+str(i))
        board = '.x' + '.' * (i-2)
        result = subprocess.run([command, board, toplay], capture_output=True, text=True)
        if (result.stderr):
            print(result.stderr)
        output = "boardsize: 1x" + str(i) + "\n" + result.stdout +"\n"
        
        with open("results.txt", "a") as f:
            f.write(output)


if __name__ == "__main__":
    main()
