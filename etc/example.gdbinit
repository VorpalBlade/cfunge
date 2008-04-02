define tr9
set variable SettingTraceLevel=9
end

document tr9
tr9
Set funge trace level to 9
end


define brkcell
break ExecuteInstruction if (ip->position.x == $arg0) && (ip->position.y == $arg1)
end

document brkcell
brkcell x y
Break when any IP reach that specific cell.
end


define brkinst
break ExecuteInstruction if (opcode == $arg0)
end

document brkinst
brkinst 'char'
Break when the given instruction is executed.
end
