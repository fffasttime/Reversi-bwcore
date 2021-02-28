from subprocess import Popen, PIPE
import time
import argparse

verbose=False

def debugprint(str, always=False):
    if verbose or always:
        if type(str)==bytes:
            str=str.decode()
        print(str.strip())

def runGame(cmd_bot1, cmd_bot2, cmd_judger):
    try:
        p_judge=Popen(cmd_judger, stdin=PIPE, stdout=PIPE)
        p_bot1=Popen(cmd_bot1, stdin=PIPE, stdout=PIPE)
        p_bot2=Popen(cmd_bot2, stdin=PIPE, stdout=PIPE)

        p_bot1.stdin.write(b'1\n')
        p_bot2.stdin.write(b'1\n')

        while True:
            # read from judger
            debugprint('[judger]')
            s_judge_cmd=p_judge.stdout.readline()
            debugprint(s_judge_cmd)
            s_judge_data=p_judge.stdout.readline()
            debugprint(s_judge_data)
            if (s_judge_cmd.strip()==b'finish'):
                winner=s_judge_data
                debugprint(p_judge.stdout.readline(), True) # reason
                break
            
            # bot run
            p_bot1.stdin.write(s_judge_data)
            p_bot1.stdin.flush()
            debugprint('[bot1]')
            resp1 = p_bot1.stdout.readline()
            if verbose:
                debugprint(resp1)
            else:
                print(f'({resp1.strip().decode()})',end=' ', flush=True)

            debugprint(p_bot1.stdout.readline()) # debug
            while (p_bot1.stdout.readline().strip()!=b'>>>BOTZONE_REQUEST_KEEP_RUNNING<<<'): pass

            p_judge.stdin.write(resp1)
            p_judge.stdin.flush()

            # read from judger
            debugprint('[judger]')
            s_judge_cmd=p_judge.stdout.readline()
            debugprint(s_judge_cmd)
            s_judge_data=p_judge.stdout.readline()
            debugprint(s_judge_data)
            if (s_judge_cmd.strip()==b'finish'):
                winner=s_judge_data
                debugprint(p_judge.stdout.readline(), True) # reason
                break

            # bot run
            p_bot2.stdin.write(s_judge_data)
            p_bot2.stdin.flush()
            debugprint('[bot2]')
            resp2 = p_bot2.stdout.readline()
            if verbose:
                debugprint(resp2)
            else:
                print(f'({resp2.strip().decode()})',end=' ', flush=True)

            debugprint(p_bot2.stdout.readline()) # debug
            while (p_bot2.stdout.readline().strip()!=b'>>>BOTZONE_REQUEST_KEEP_RUNNING<<<'): pass

            p_judge.stdin.write(resp2)
            p_judge.stdin.flush()
        
        winner=int(winner.decode().strip())
        print('winner: ', winner)
        return winner
    
    finally:
        p_judge.kill()
        p_bot1.kill()
        p_bot2.kill()

def runTests(bot1, bot2, judger, n_game):
    win_black, draw_black = 0, 0
    win_white, draw_white = 0, 0

    for i in range(n_game):
        print("Game ", i)
        winner=runGame(bot1, bot2, judger)
        if winner==0: win_black+=1
        elif winner==2: draw_black+=1
    loss_black=n_game-win_black-draw_black
    
    for i in range(n_game):
        print("Game ", i)
        winner=runGame(bot2, bot1, judger)
        if winner==1: win_white+=1
        elif winner==2: draw_white+=1
    loss_white=n_game-win_white-draw_white
    
    with open("match.log", "a") as f:
        f.write(f"Match finished at {time.ctime(time.time())}\n")
        f.write(f"bot1:{bot1} bot2:{bot2}\n")
        f.write("bot1 | win | loss | draw | score\n")
        f.write(f"as_b | {win_black:^3d} | {loss_black:^4d} | {draw_black:^4d} | ")
        f.write(f"{win_black+draw_black/2:^4.1f} / {n_game}\n")
        f.write(f"as_w | {win_white:^3d} | {loss_white:^4d} | {draw_white:^4d} | ")
        f.write(f"{win_white+draw_white/2:^4.1f} / {n_game}\n")
        f.write(f"{win_black+draw_black/2+win_white+draw_white/2:^4.1f} / {n_game*2}\n")
        f.write('\n')

if __name__ == '__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('bot1',default='botzone.exe')
    parser.add_argument('bot2',default='botzone.exe')
    parser.add_argument('n_game', default=50, help='games play each side')
    parser.add_argument('-j', '--judger', default='judger.exe')
    parser.add_argument('-v', '--verbose', action='store_true', help='show communication')
    args=parser.parse_args()
    if (args.verbose):
        verbose=True
    runTests(args.bot1, args.bot2, args.judger, args.n_game)
