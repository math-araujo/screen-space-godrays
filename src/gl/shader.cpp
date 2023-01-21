#include "shader.hpp"

#include <array>
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gl
{

Shader::Shader(const std::string& shader_source_code, Type type) : identifier_{glCreateShader(to_underlying(type))}
{
    const char* source_code_ptr = shader_source_code.c_str();
    glShaderSource(identifier_, 1, &source_code_ptr, nullptr);
    glCompileShader(identifier_);
    check_shader_compilation(identifier_, shader_typename(type));
}

void check_shader_compilation(std::uint32_t shader_id, std::string_view shader_type)
{
    int compilation_successful = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compilation_successful);

    if (!compilation_successful)
    {
        std::array<char, 1024> error_log{};
        glGetShaderInfoLog(shader_id, static_cast<GLsizei>(error_log.size()), nullptr, error_log.data());
        std::stringstream stream;
        stream << shader_type << " Shader compilation error:\n" << error_log.data() << "\n";
        throw std::runtime_error(stream.str());
    }
}

Shader::Shader(Shader&& shader) noexcept : identifier_{shader.identifier_}
{
    shader.identifier_ = 0;
}

Shader& Shader::operator=(Shader&& shader) noexcept
{
    std::swap(identifier_, shader.identifier_);
    return *this;
}

Shader::~Shader()
{
    glDeleteShader(identifier_);
}

std::uint32_t Shader::identifier() const
{
    return identifier_;
}

const std::string& Shader::shader_typename(Shader::Type type)
{
    static const std::unordered_map<Shader::Type, std::string> shader_types = {
        {Shader::Type::Vertex, "Vertex"},
        {Shader::Type::TessControl, "Tessellation Control"},
        {Shader::Type::TessEval, "Tessellation Evaluation"},
        {Shader::Type::Fragment, "Fragment"},
        {Shader::Type::Compute, "Compute"},
    };
    return shader_types.at(type);
}

ShaderProgram::ShaderProgram(std::initializer_list<ShaderInfo> initializer) : program_id_{glCreateProgram()}
{
    std::vector<Shader> shaders;
    shaders.reserve(initializer.size());

    for (const ShaderInfo& shader_info : initializer)
    {
        shaders.emplace_back(load_shader_from_file(shader_info));
        glAttachShader(program_id_, shaders.back().identifier());
    }

    glLinkProgram(program_id_);
    check_shader_program_link_status(program_id_, initializer);

    for (const auto& shader : shaders)
    {
        glDetachShader(program_id_, shader.identifier());
    }

    retrieve_uniforms();
}

Shader load_shader_from_file(const ShaderInfo& shader_info)
{
    std::ifstream shader_file{shader_info.filepath.data()};
    if (!shader_file.is_open())
    {
        std::stringstream error_log_stream;
        error_log_stream << "File " << shader_info.filepath << " could not be opened";
        throw std::runtime_error(error_log_stream.str());
    }

    std::stringstream source_code_stream;
    source_code_stream << shader_file.rdbuf();
    return Shader{process_shader_preprocessor_directives(source_code_stream.str(), shader_info), shader_info.type};
}

std::string process_shader_preprocessor_directives(std::string shader_source, const ShaderInfo& shader_info)
{
    // Injects #define directive
    std::string define_directives;
    for (const std::string& define : shader_info.define_variables)
    {
        define_directives += "#define " + define + "\n";
    }

    if (!define_directives.empty())
    {
        // The preprocessor "#version" must necessarily be on the first line of the
        // shader code. Therefore, the define directives must appear after it.
        const std::size_t version_preprocessor_end_pos{shader_source.find('\n')};
        shader_source.insert(version_preprocessor_end_pos + 1, define_directives);
    }
    /*
    This functions performs a search for a include directive pattern,
    such as "#include shader_file".
    If there's a include directive, the corresponding header (e.g. "shader_file")
    is read from file and replaces the include directive by the shader file contents.
    */
    const std::filesystem::path shader_path{shader_info.filepath};
    const std::string_view pattern{"#include "};
    const std::size_t include_directive_position{shader_source.find(pattern)};
    if (include_directive_position != std::string::npos)
    {
        const std::size_t include_shader_start_pos{include_directive_position + pattern.size()};
        const std::size_t include_shader_end_pos{shader_source.find('\n', include_shader_start_pos)};
        std::string filename =
            shader_source.substr(include_shader_start_pos, include_shader_end_pos - include_shader_start_pos - 1);
        std::filesystem::path include_shader_path{shader_path.parent_path()};
        include_shader_path.append(filename);
        std::ifstream shader_file{include_shader_path};
        std::stringstream source_code_stream;
        source_code_stream << shader_file.rdbuf();
        auto include_pattern =
            shader_source.substr(include_directive_position, include_shader_end_pos - include_directive_position);
        return std::regex_replace(shader_source, std::regex(include_pattern), source_code_stream.str());
    }

    return shader_source;
}

void check_shader_program_link_status(std::uint32_t shader_program_id, std::initializer_list<ShaderInfo> shader_data)
{
    int linking_success{0};
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &linking_success);
    if (!linking_success)
    {
        std::array<char, 1024> error_log{};
        glGetProgramInfoLog(shader_program_id, static_cast<GLsizei>(error_log.size()), nullptr, error_log.data());
        std::stringstream stream;
        stream << "Shader program linking error:\n" << error_log.data() << "\nShader Program Files: ";

        for (const auto& [filepath, shader_type, defines] : shader_data)
        {
            stream << filepath << " ";
        }
        stream << "\n";
        throw std::runtime_error(stream.str());
    }
}

void ShaderProgram::retrieve_uniforms()
{
    int number_of_uniforms{0};
    glGetProgramInterfaceiv(program_id_, GL_UNIFORM, GL_ACTIVE_RESOURCES, &number_of_uniforms);
    std::array<GLenum, 2> properties{GL_NAME_LENGTH, GL_LOCATION};
    std::array<GLint, 2> results{};
    std::vector<char> uniform_name(256);
    for (int uniform = 0; uniform < number_of_uniforms; ++uniform)
    {
        glGetProgramResourceiv(program_id_, GL_UNIFORM, uniform, properties.size(), properties.data(), results.size(),
                               nullptr, results.data());

        // Get resources (uniform name and uniform location)
        uniform_name.resize(results[0]);
        glGetProgramResourceName(program_id_, GL_UNIFORM, uniform, uniform_name.size(), nullptr, uniform_name.data());
        std::uint32_t uniform_location = results.back();

        // The name returned contains a null-terminator, so it's necessary to read uniform_name.size() - 1 characters
        uniform_locations.emplace(std::string{uniform_name.data(), uniform_name.size() - 1}, uniform_location);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept : program_id_{other.program_id_}
{
    other.program_id_ = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    std::swap(program_id_, other.program_id_);
    return *this;
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(program_id_);
}

void ShaderProgram::use()
{
    glUseProgram(program_id_);
}

void ShaderProgram::set_bool_uniform(const std::string& uniform_name, bool value)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform1i(program_id_, uniform_locations[uniform_name], static_cast<int>(value));
}

void ShaderProgram::set_int_uniform(const std::string& uniform_name, int value)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform1i(program_id_, uniform_locations[uniform_name], value);
}

void ShaderProgram::set_int_array_uniform(const std::string& uniform_name, const int* value, std::size_t count)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform1iv(program_id_, uniform_locations[uniform_name], count, value);
}

void ShaderProgram::set_float_uniform(const std::string& uniform_name, float value)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform1f(program_id_, uniform_locations[uniform_name], value);
}

void ShaderProgram::set_float_array_uniform(const std::string& uniform_name, const float* value, std::size_t count)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform1fv(program_id_, uniform_locations[uniform_name], count, value);
}

void ShaderProgram::set_vec2_uniform(const std::string& uniform_name, float x, float y)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform2f(program_id_, uniform_locations[uniform_name], x, y);
}

void ShaderProgram::set_vec2_uniform(const std::string& uniform_name, const glm::vec2& vector)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform2fv(program_id_, uniform_locations[uniform_name], 1, glm::value_ptr(vector));
}

void ShaderProgram::set_vec2_array_uniform(const std::string& uniform_name, const std::vector<glm::vec2>& vec2_array)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform2fv(program_id_, uniform_locations[uniform_name], static_cast<std::uint32_t>(vec2_array.size()),
                        glm::value_ptr(vec2_array.front()));
}

void ShaderProgram::set_vec3_uniform(const std::string& uniform_name, float x, float y, float z)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform3f(program_id_, uniform_locations[uniform_name], x, y, z);
}

void ShaderProgram::set_vec3_uniform(const std::string& uniform_name, const glm::vec3& vector)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform3fv(program_id_, uniform_locations[uniform_name], 1, glm::value_ptr(vector));
}

void ShaderProgram::set_vec3_array_uniform(const std::string& uniform_name, const std::vector<glm::vec3>& vec3_array)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform3fv(program_id_, uniform_locations[uniform_name], static_cast<std::uint32_t>(vec3_array.size()),
                        glm::value_ptr(vec3_array.front()));
}

void ShaderProgram::set_vec4_uniform(const std::string& uniform_name, const glm::vec4& vector)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform4fv(program_id_, uniform_locations[uniform_name], 1, glm::value_ptr(vector));
}

void ShaderProgram::set_vec4_array_uniform(const std::string& uniform_name, const std::vector<glm::vec4>& vec4_array)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniform4fv(program_id_, uniform_locations[uniform_name], static_cast<std::uint32_t>(vec4_array.size()),
                        glm::value_ptr(vec4_array.front()));
}

void ShaderProgram::set_mat4_uniform(const std::string& uniform_name, const glm::mat4& matrix)
{
    assert(uniform_locations.contains(uniform_name));
    glProgramUniformMatrix4fv(program_id_, uniform_locations[uniform_name], 1, GL_FALSE, glm::value_ptr(matrix));
}

} // namespace gl