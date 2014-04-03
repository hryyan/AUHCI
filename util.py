import os
import os.path
import ntpath

src = "D:/ck/cohn-kanade/cohn-kanade"
dst = "C:/Users/vincent/Documents/Visual Studio 2010/Projects/CV_64bit/FacialExpression_x64"

src_image_num = 0
dst_image_num = 0

src_dir_outter_list = os.listdir(src)
dst_dir_outter_list = os.listdir(dst)

src_dir_outter_list = [f for f in src_dir_outter_list
                        if os.path.isdir(f) and f[0] == 'S']
dst_dir_outter_list = [f for f in dst_dir_outter_list
                        if os.path.isdir(f) and f[0] == 'S']

def cmp(path_name, dir_name):
    src_image_list = os.listdir(src+'/'+path_name+'/'+dir_name)
    try:
        dst_image_list = os.listdir(dst+'/'+path_name+'/'+dir_name)
    except Exception, e:
        print dst+'/'+path_name+'/'+dir_name+"can't find in dst"
        return 0

    src_image_list = [i for i in src_image_list
                    if os.path.isfile(src+'/'+path_name+'/'+dir_name+'/'+i)]
    dst_image_list = [i for i in dst_image_list
                    if os.path.isfile(dst+'/'+path_name+'/'+dir_name+'/'+i)]

    global src_image_num
    src_image_num = src_image_num + len(src_image_list)
    global dst_image_num
    dst_image_num = dst_image_num + len(dst_image_list)

    if len(src_image_list) == len(dst_image_list):
        return 0
    else:
        return len(src_image_list) - len(dst_image_list)

for src_dir in src_dir_outter_list:
    src_dir_inner_list = os.listdir(src+'/'+src_dir)
    for dir_name in src_dir_inner_list:
        different = cmp(src_dir, dir_name)
        print "dir: "+src_dir+" dir: "+dir_name+" dif: "+str(different)

print "there are images in src"
print src_image_num
print "there are images in dst"
print dst_image_num
# print src_dir_outter_list
# print dst_dir_outter_list

raw_input()