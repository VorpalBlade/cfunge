define tr9
set variable setting_trace_level=9
end

document tr9
Set funge trace level to 9
end


define brkcell
break execute_instruction if (ip->position.x == $arg0) && (ip->position.y == $arg1)
end

document brkcell
Break when any IP reach that specific cell.
brkcell x y
end


define brkinst
break execute_instruction if (opcode == $arg0)
end

document brkinst
Break when the given instruction is executed.
brkinst 'char'
end
