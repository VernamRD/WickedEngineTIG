assert(arg[1] == "-a", "Expected -a flag")
local out_cpp   = arg[2]
local out_types = arg[3]
local header    = arg[4]

local function parse_blocks(content: string)
    local root = { kind = "root", name = "", children = {}, body = "" }
    local stack = { root }

    local lines: {string} = {}
    for line in content:gmatch("[^\n]+") do
        table.insert(lines, line)
    end

    local pending_luau: {string: any}? = nil
    local prev_trimmed = ""  -- предыдущая непустая строка

    local i = 1
    while i <= #lines do
        local line = lines[i]
        local trimmed = line:match("^%s*(.-)%s*$")

        -- Skip empty lines but remember context
        if trimmed == "" then
            i += 1
            continue
        end

        -- { check before } to handle inline blocks correctly
        if trimmed:find("{") then
            -- inline: { и } на одной строке — пропускаем
            if trimmed:find("}") then
                prev_trimmed = trimmed
                i += 1
                continue
            end

            -- Заголовок блока: либо на этой строке до {, либо это была prev_trimmed
            local header = trimmed:gsub("{.*$", ""):match("^%s*(.-)%s*$")
            if header == "" then
                -- { стоит отдельно — заголовок это предыдущая строка
                header = prev_trimmed
            end

            local kind = "block"
            local name = header

            if header:match("^namespace%s+") then
                kind = "namespace"
                name = header:match("^namespace%s+(.+)$")
            elseif header:match("^struct%s+") then
                kind = "struct"
                name = header:match("^struct%s+([%w_]+)")
            elseif header:match("^class%s+") then
                kind = "class"
                name = header:match("^class%s+([%w_]+)")
            elseif header:match("^enum%s+") then
                kind = "enum"
                name = header:match("^enum%s*class%s+([%w_]+)") 
                    or header:match("^enum%s+([%w_]+)")
            end

            local node = {
                kind       = kind,
                name       = name or header,
                header     = header,
                attributes = pending_luau,
                children   = {},
            }
            pending_luau = nil

            table.insert(stack[#stack].children, node)
            table.insert(stack, node)

        elseif trimmed:find("^}") then
            if #stack > 1 then
                table.remove(stack)
            end

        else
            -- Обычное объявление
            local current = stack[#stack]
            if pending_luau then
                table.insert(current.children, {
                    kind        = "declaration",
                    name        = trimmed,
                    declaration = trimmed,
                    attributes  = pending_luau,
                    children    = {},
                })
                pending_luau = nil
            end
        end

        prev_trimmed = trimmed
        i += 1
    end

    return root
end

local function print_tree(node, indent: number)
    indent = indent or 0
    local prefix = string.rep("  ", indent)
    print(prefix .. "[" .. node.kind .. "] " .. (node.name or "?"))

    if node.attributes then
        for k, v in pairs(node.attributes) do
            print(prefix .. "  @luau " .. k .. " = " .. tostring(v))
        end
    end

    if node.declaration then
        print(prefix .. "  decl: " .. node.declaration)
    end

    for _, child in ipairs(node.children) do
        print_tree(child, indent + 1)
    end
end

print ("Generate " .. header)
local file_content = io.readfile(header)
local tree = parse_blocks(file_content)
print_tree(tree, 0)