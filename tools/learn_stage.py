from stat import FILE_ATTRIBUTE_COMPRESSED
import sys, os
import platform

if platform.system()=="Windows":
    prefix = ''
else:
    prefix = './'
folder = 'rawdata3'
old_folder = 'rawdata2'
default_games = 900000

start_stage = int(input('start_stage: '))
first_stage_config = input('first_stage_play(default 900000): ')
first_stage_play = default_games
if first_stage_config != '':
    first_stage_play = int(first_stage_config)


for stage in range(start_stage, 11):
    print('------------------------stage %d---------------------------'%stage, flush=True)

    ngames = first_stage_play if stage==start_stage else default_games

    os.system('%sgendata %s %d %d'%(prefix, folder, stage, ngames))
    os.system('%slinreg %s %d'%(prefix, folder, stage))
    os.system('%slinreg concat %s %s %d'%(prefix, folder, old_folder, stage))
    
    print('\n\n', flush=True)
