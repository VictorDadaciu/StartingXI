from enum import Enum
from typing import Any

class TokenType(Enum):
    NUM = 0
    LAYOUT = 1
    OPEN_PAR = 2
    CLOSED_PAR = 3
    SET = 4
    BINDING = 5
    IN = 6
    OUT = 7
    UNIFORM = 8
    OPEN_BRACKET = 9
    CLOSED_BRACKET = 10
    COMMA = 11 
    SEMICOLON = 12
    WORD = 13
    EQ = 14
    LOCATION = 15
    OPEN_SQ = 16
    CLOSED_SQ = 17
    TYPE = 18
    HASH = 19
    COLON = 20
    STRUCT = 21

VALUE_TOKENS = {
    TokenType.NUM,
    TokenType.WORD,
    TokenType.TYPE,
}

GLSL_TYPES: set[str] = {
    "void",
    "bool",
    "int",
    "uint",
    "float",
    "double",
    "bvec2",
    "bvec3",
    "bvec4",
    "ivec2",
    "ivec3",
    "ivec4",
    "uvec2",
    "uvec3",
    "uvec4",
    "vec2",
    "vec3",
    "vec4",
    "dvec2",
    "dvec3",
    "dvec4",
    "mat2",
    "mat3",
    "mat4",
    "dmat2",
    "dmat3",
    "dmat4",
    "sampler1D",
    "sampler2D",
    "sampler3D",
}

SIMPLE_TOKENS: dict[str, TokenType] = {
    '(': TokenType.OPEN_PAR,
    ')': TokenType.CLOSED_PAR,
    '[': TokenType.OPEN_SQ,
    ']': TokenType.CLOSED_SQ,
    '{': TokenType.OPEN_BRACKET,
    '}': TokenType.CLOSED_BRACKET,
    ';': TokenType.SEMICOLON,
    ',': TokenType.COMMA,
    '=': TokenType.EQ,
    '#': TokenType.HASH,
    ':': TokenType.COLON,
}

KEYWORDS: dict[str, TokenType] = {
    "layout": TokenType.LAYOUT,
    "set": TokenType.SET,
    "binding": TokenType.BINDING,
    "uniform": TokenType.UNIFORM,
    "location": TokenType.LOCATION,
    "in": TokenType.IN,
    "out": TokenType.OUT,
    "struct": TokenType.STRUCT,
}

class Token:
    def __init__(self, type: TokenType, content: None | int | float | str = None) -> None:
        self.type = type
        self.content = content
        assert self.type not in VALUE_TOKENS or self.content is not None

    def __str__(self) -> str:
        return f"<{self.type.name}, {self.content}>" if self.type in VALUE_TOKENS else f"<{self.type.name}>"
    
    def __repr__(self) -> str:
        return self.__str__()
    
    def __eq__(self, value: object) -> bool:
        if isinstance(value, self.__class__):
            return self.type == value.type
        elif isinstance(value, TokenType):
            return self.type == value
        return False

def is_int(token: str) -> bool:
    try:
        int(token)
        return True
    except ValueError:
        return False

def is_float(token: str) -> bool:
    try:
        float(token)
        return True
    except ValueError:
        return False

def lex(file_path: str) -> list[Token]:
    tokens: list[Token] = []
    with open(file_path) as file:
        for line in file:
            buf: list[str] = []
            for col in range(len(line)):
                c = line[col]
                if c.isspace() or c in SIMPLE_TOKENS:
                    if len(buf) > 0:
                        tok: str = ''.join(buf)
                        buf = []
                        if tok == "//":
                            break # TODO: ignore comments for now
                        elif tok in KEYWORDS:
                            tokens.append(Token(KEYWORDS[tok]))
                        elif tok in GLSL_TYPES:
                            tokens.append(Token(TokenType.TYPE, tok))
                        elif is_int(tok):
                            tokens.append(Token(TokenType.NUM, int(tok)))
                        elif is_float(tok):
                            tokens.append(Token(TokenType.NUM, float(tok)))
                        else:
                            tokens.append(Token(TokenType.WORD, tok))
                    if c in SIMPLE_TOKENS:
                        tokens.append(Token(SIMPLE_TOKENS[c]))
                    else:
                        continue
                else:
                    buf.append(c)
        return tokens

class ExtensionMode(Enum):
    ENABLE = 0,
    REQUIRE = 1,
    DISABLE = 2,
    WARN = 3

EXTENSION_MODES_DICT: dict[str, ExtensionMode] = {
    "enable": ExtensionMode.ENABLE,
    "require": ExtensionMode.REQUIRE,
    "disable": ExtensionMode.DISABLE,
    "warn": ExtensionMode.WARN,
}

class SeqType(Enum):
    # #
    PREPROCESSOR_START = 0
    # layout(
    LAYOUT_BEGIN = 1
    # version <int>
    VERSION_DIRECTIVE = 2
    # extension <extension> : <extension_mode>
    EXTENSION_DIRECTIVE = 3
    # location = <int>) in
    LOCATION_IN = 4
    # <glsl_type> <var_name>;
    GLSL_TYPE_DECL = 5
    # binding = <int>) uniform
    BINDING = 6
    # set = <int>, binding = <int>) uniform
    SET_BINDING = 7
    # struct
    STRUCT_DECL = 8
    # }
    END_SCOPE = 9
    # <type_name> {
    TYPE_DEF = 10
    # <type_name> <var_name>;
    CUSTOM_TYPE_DECL = 11

SEQUENCES: dict[SeqType, list[TokenType]] = {
    SeqType.PREPROCESSOR_START:     [TokenType.HASH],
    SeqType.LAYOUT_BEGIN:           [TokenType.LAYOUT, TokenType.OPEN_PAR],
    SeqType.VERSION_DIRECTIVE:      [TokenType.WORD, TokenType.NUM],
    SeqType.EXTENSION_DIRECTIVE:    [TokenType.WORD, TokenType.WORD, TokenType.COLON, TokenType.WORD],
    SeqType.LOCATION_IN:            [TokenType.LOCATION, TokenType.EQ, TokenType.NUM, TokenType.CLOSED_PAR, TokenType.IN],
    SeqType.GLSL_TYPE_DECL:         [TokenType.TYPE, TokenType.WORD, TokenType.SEMICOLON],
    SeqType.BINDING:                [TokenType.BINDING, TokenType.EQ, TokenType.NUM, TokenType.CLOSED_PAR, TokenType.UNIFORM],
    SeqType.SET_BINDING:            [TokenType.SET, TokenType.EQ, TokenType.NUM, TokenType.COMMA, TokenType.BINDING, TokenType.EQ, TokenType.NUM, TokenType.CLOSED_PAR, TokenType.UNIFORM],
    SeqType.STRUCT_DECL:            [TokenType.STRUCT],
    SeqType.END_SCOPE:              [TokenType.CLOSED_BRACKET],
    SeqType.TYPE_DEF:               [TokenType.WORD, TokenType.OPEN_BRACKET],
    SeqType.CUSTOM_TYPE_DECL:       [TokenType.WORD, TokenType.WORD, TokenType.SEMICOLON],
}

class DataType:
    def __init__(self, type: str | list[str]) -> None:
        self.type: str | list[str] = type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, DataType):
            return False
        return self.type == other.type

    def __str__(self) -> str:
        return str(self.type)

    def __repr__(self) -> str:
        return self.__str__()
    
    def __hash__(self) -> int:
        if isinstance(self.type, str):
            return hash(self.type)
        else:
            return hash(tuple(self.type))

struct_aliases: dict[DataType, list[str]] = {}

class DescriptorSetLayout:
    def __init__(self) -> None:
        self.descriptors: dict[int, DataType] = {}

    def __or__(self, other: Any):
        if not isinstance(other, DescriptorSetLayout):
            return NotImplemented
        new: DescriptorSetLayout = DescriptorSetLayout()
        new.descriptors = self.descriptors | other.descriptors
        return new

def tabs(n: int) -> str:
    return '\t' * n

class ShaderModuleInfo:
    def __init__(self) -> None:
        self.version: int = 0
        self.extensions: dict[str, ExtensionMode] = {}
        self.inputs: dict[int, DataType] = {}
        self.structs: dict[str, DataType] = {}
        self.descriptor_set_layouts: dict[int, DescriptorSetLayout] = {}

    def __str__(self) -> str:
        # version
        ret_val: str = tabs(1) + f"version: {self.version},\n"
        ret_val += tabs(1) + "extensions: {\n"
        flipped = {v: k for k, v in self.extensions.items()}
        for ext, mode in self.extensions.items():
            ret_val += tabs(2) + f"{ext}: {flipped[mode]}"
        ret_val += tabs(1) + "},\n"

        # inputs
        ret_val += tabs(1) + "inputs: {\n"
        for loc, type in self.inputs.items():
            ret_val += tabs(2) + f"{loc}: {type.type},\n"
        ret_val += tabs(1) + "},\n"

        # structs
        ret_val += tabs(1) + "structs: {\n"
        for type_name, struct in self.structs.items():
            ret_val += tabs(2) + f"{type_name}: {struct}\n"
        ret_val += tabs(1) + "},\n"

        # descriptor set layouts
        ret_val += tabs(1) + "descriptor set layouts: {\n"
        for dset, dsl in self.descriptor_set_layouts.items():
            ret_val += tabs(2) + f"set {dset}: " + "{\n"
            for binding, type in dsl.descriptors.items():
                ret_val += tabs(3) + f"binding {binding}: {type.type},\n"
            ret_val += tabs(2) + "},\n"
        ret_val += tabs(1) + "},\n"
        return ret_val
    
    def __repr__(self) -> str:
        return self.__str__()

class Parser:
    def __init__(self, shader_module: str) -> None:
        self.tokens: list[Token] = lex(shader_module)
        self.info: ShaderModuleInfo = ShaderModuleInfo()

        print(f"    Parsing {shader_module}...")
        while not self.is_empty():
            if len(self.seq_matches(*SEQUENCES[SeqType.PREPROCESSOR_START])) > 0 and self.handle_preprocessor_directive():
                continue
            elif len(self.seq_matches(*SEQUENCES[SeqType.LAYOUT_BEGIN])) > 0 and self.handle_layout_info():
                continue
            elif len(self.seq_matches(*SEQUENCES[SeqType.STRUCT_DECL])) > 0 and self.handle_custom_type_definition():
                continue
            self.skip()
        assert len(self.tokens) == 0
        print(self.info)

    def handle_preprocessor_directive(self) -> bool:
        if len(seq := self.seq_matches(*SEQUENCES[SeqType.VERSION_DIRECTIVE])) > 0:
            if seq[0].content == "version" and isinstance(seq[1].content, int):
                self.info.version = seq[1].content
                return True
        elif len(seq := self.seq_matches(*SEQUENCES[SeqType.EXTENSION_DIRECTIVE])) > 0:
            if seq[0].content == "extension" and seq[3].content in EXTENSION_MODES_DICT:
                self.info.extensions[str(seq[1].content)] = EXTENSION_MODES_DICT[str(seq[3].content)]
                return True
        return False
    
    def handle_layout_info(self) -> bool:
        if len(seq := self.seq_matches(*SEQUENCES[SeqType.LOCATION_IN])) > 0 and isinstance(seq[2].content, int):
            loc: int = seq[2].content
            if len(seq := self.seq_matches(*SEQUENCES[SeqType.GLSL_TYPE_DECL])) > 0:
                self.info.inputs[loc] = DataType(str(seq[0].content))
                return True
        elif len(seq := self.seq_matches(*SEQUENCES[SeqType.BINDING])) > 0 and isinstance(seq[2].content, int):
            return self.handle_uniform_declaration(0, seq[2].content)
        elif len(seq := self.seq_matches(*SEQUENCES[SeqType.SET_BINDING])) > 0 and isinstance(seq[2].content, int) and isinstance(seq[6].content, int):
            return self.handle_uniform_declaration(seq[2].content, seq[6].content)
        return False
    
    def handle_uniform_declaration(self, dset: int, binding: int) -> bool:
        if self.info.descriptor_set_layouts.get(dset) is None:
            self.info.descriptor_set_layouts[dset] = DescriptorSetLayout()
        if len(seq := self.seq_matches(*SEQUENCES[SeqType.GLSL_TYPE_DECL])) > 0 or \
            (len(seq := self.seq_matches(*SEQUENCES[SeqType.CUSTOM_TYPE_DECL])) > 0 and seq[0].content in self.info.structs):
            type: DataType = DataType(str(seq[0].content))
        else:
            type: DataType = self.info.structs[self.handle_custom_type_definition()]
            self.skip(2)
        self.info.descriptor_set_layouts[dset].descriptors[binding] = type
        return True

    def handle_custom_type_definition(self) -> str:
        type_list: list[str] = []
        type_name: str = str(self.seq_matches(*SEQUENCES[SeqType.TYPE_DEF])[0].content)
        while len(self.seq_matches(*SEQUENCES[SeqType.END_SCOPE])) == 0:
            seq = self.seq_matches(*SEQUENCES[SeqType.GLSL_TYPE_DECL])
            type_list.append(str(seq[0].content))

        struct: DataType = DataType(type_list)
        global struct_aliases
        if struct_aliases.get(struct) is None:
            struct_aliases[struct] = [type_name]
        else:
            struct_aliases[struct].append(type_name)
        self.info.structs[type_name] = struct
        return type_name

    # Trims beginning of token list if args match
    def seq_matches(self, *args: TokenType) -> list[Token]:
        n: int = len(args)
        if n > len(self.tokens):
            return []
        
        for i in range(n):
            if self.tokens[i] != args[i]:
                return []
        
        # sequence matches, trim and return
        seq: list[Token] = self.tokens[:n]
        self.tokens = self.tokens[n:]
        return seq
    
    def skip(self, n: int = 1) -> None:
        self.tokens = self.tokens[n:]
    
    def is_empty(self) -> bool:
        return len(self.tokens) == 0
    
SHADER_MODULES_ORDER: list[str] = ["vert", "frag"]

class PipelineInfo:
    def __init__(self, pipeline_name: str, modules: dict[str, ShaderModuleInfo]) -> None:
        print(f"    Parsing pipeline \"{pipeline_name}\"...")
        self.modules = modules

        self.inputs = self.modules["vert"].inputs
        self.descriptor_set_layouts: dict[int, DescriptorSetLayout] = {}
        for _, module_info in modules.items():
            for dset, dsl in module_info.descriptor_set_layouts.items():
                if self.descriptor_set_layouts.get(dset) is None:
                    self.descriptor_set_layouts[dset] = dsl
                else:
                    self.descriptor_set_layouts[dset] = self.descriptor_set_layouts[dset] | dsl
    
    def __str__(self) -> str:
        # inputs
        ret_val: str = tabs(1) + "inputs: {\n"
        for loc, type in self.inputs.items():
            ret_val += tabs(2) + f"{loc}: {type.type},\n"
        ret_val += tabs(1) + "},\n"

        # descriptor set layouts
        ret_val += tabs(1) + "descriptor set layouts: {\n"
        for dset, dsl in self.descriptor_set_layouts.items():
            ret_val += tabs(2) + f"set {dset}: " + "{\n"
            for binding, type in dsl.descriptors.items():
                ret_val += tabs(3) + f"binding {binding}: {type.type},\n"
            ret_val += tabs(2) + "},\n"
        ret_val += tabs(1) + "},\n"
        return ret_val
    
    def __repr__(self) -> str:
        return self.__str__()
