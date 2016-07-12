# -*- coding: utf-8 -*-
__author__ = 'xiazeng'


import json

def read_git_lines(content):
    # Index: app/src/com/tencent/mm/plugin/base/stub/OAuthUI.java
    # ===================================================================
    # --- app/src/com/tencent/mm/plugin/base/stub/OAuthUI.java    (版本 580019)
    # +++ app/src/com/tencent/mm/plugin/base/stub/OAuthUI.java    (版本 581062)
    # @@ -323,19 +323,19 @@
     #            }
    #
     #            if (!networkAvailable) {
    # -                    String html = null;
    # +                    String errUrl = null;
    # @@ -343,19 +343,19 @@
    # ...
    #
    # change lines: 326, ...
    # delete lines: 326, ...
    lines = [l.strip() for l in content.split("\n")]
    changelinemap = {}
    index = 0
    while index+2 < len(lines):
        line = lines[index+2]
        filename = line[len('--- a/'):]
        index = index+2
        changed_lines = []

        if index >= len(lines):
            break
        while index<len(lines) and not lines[index].startswith('diff --git ') and not lines[index].startswith('@@'):
            index += 1
        while index < len(lines) and not lines[index].startswith('diff --git '):
            line = lines[index]
            if line.startswith("@@"):
                # header
                begin_idx = line.index('+')+1
                if ',' in line[begin_idx:]:
                    end_idx = line.index(',',begin_idx)
                else:
                    end_idx = line.index(' ',begin_idx)
                begin_line_idx = int(line[begin_idx:end_idx])
                delta = 0
            elif line.startswith('-'):
                # removed lines
                tmp = ''
            elif line.startswith('+'):
                # changed lines
                changed_lines.append(begin_line_idx+delta)
                delta = delta + 1
            else:
                delta = delta + 1
            # next line
            index = index + 1

        # record changed lines
        filename = filename.strip()
        print "filename:",index, filename
        if filename.endswith('.m') or filename.endswith('.mm') or filename.endswith(".cpp"):
            changelinemap[filename] = changed_lines
    return changelinemap

if "__main__" == __name__:
    import sys
    if len(sys.argv) == 2:
        content = open(sys.argv[1]).read()
        print read_git_lines(content)
    if len(sys.argv) == 3:
        #get one file diff lines
        content = open(sys.argv[1]).read()
        result = read_git_lines(content)
        lines_list = result.values()
        if lines_list:
            lines = lines_list[0]
        else:
            lines = []
        json_fp = open(sys.argv[2], "w")
        print result
        json.dump(lines, json_fp)
