--
-- make_cpp.lua
-- Generate a C/C++ project makefile.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	local echo_cmd = 'env echo -e'
	local reset_color     = '\\033[0m'
	local build_color     = '\\033[0;1m'
	local compile_color   = '\\033[0;32m'
	local link_color      = '\\033[0;36m'
	local reconfig_color  = '\\033[0;33m'


	premake.make.cpp = { }
	local _ = premake.make.cpp


	local function get_project_tag(prj)
		return ('%-16s  '):format('<' .. prj.name .. '>')
	end

	local function makeconcat(x)
	  if (x) then
	    return table.concat(x, " \\\n\t")
    end
    return nil
  end

	function premake.make_foreign(prj)
		if (not prj.foreigntarget) then error("missing foreigntarget parameter") end
		if (not prj.foreignbuild)  then error("missing foreignbuild parameter")  end
		if (not prj.foreignclean)  then error("missing foreignclean parameter")  end

		local sln        = prj.solution
		local dir        = prj.foreignproject
		local target     = prj.foreigntarget
		local build      = makeconcat(prj.foreignbuild)
		local config     = makeconcat(prj.foreignconfig)
		local clean      = makeconcat(prj.foreignclean)
		local superclean = makeconcat(prj.foreignsuperclean)

		if (-1 > 0) then -- FIXME - print's
			print("slnbasedir",  sln.basedir)
			print("prjlocation", prj.location)
			print("foreignproject",    dir)
			print("foreigntarget",     target)
			print("foreignconfig",     config)
			print("foreignbuild",      build)
			print("foreignclean",      clean)
			print("foreignsuperclean", superclean)
		end

		if (config) then
  		local dirfromtop = path.getrelative(sln.basedir, path.join(prj.location, prj.foreignproject))
		  print(("-"):rep(80))
			print("Configuring " .. prj.name)
			os.executef("cd '%s' ; %s", dirfromtop, config)
		  print(("-"):rep(80))
		end

		_p("# target = " .. target)
		_p(".PHONY: clean superclean")
		_p("%s:", path.join(dir, target))
		_p("\tcd '%s' ; \\\n\t%s", dir, build)
		if (clean) then
			_p("clean:")
			_p("\tcd '%s' ; \\\n\t%s", dir, clean)
		end
	end


	function premake.make_cpp(prj)
		if (prj.foreignproject) then
			premake.make_foreign(prj)
			return
		end

		-- prefix each command with prj_tag for multi-job builds
		local prj_tag = get_project_tag(prj)

		-- create a shortcut to the compiler interface
		local cc = premake.gettool(prj)

		-- build a list of supported target platforms that also includes a generic build
		local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")

		premake.gmake_cpp_header(prj, cc, platforms)

		for _, platform in ipairs(platforms) do
			for cfg in premake.eachconfig(prj, platform) do
				premake.gmake_cpp_config(cfg, cc, prj)
			end
		end

		-- list intermediate files
		_p('OBJECTS := \\')
		for _, file in ipairs(prj.files) do
			if path.iscppfile(file) then
				_p('\t$(OBJDIR)/%s.o \\', _MAKE.esc(path.getbasename(file)))
			end
		end
		_p('')

		_p('RESOURCES := \\')
		for _, file in ipairs(prj.files) do
			if path.isresourcefile(file) then
				_p('\t$(OBJDIR)/%s.res \\', _MAKE.esc(path.getbasename(file)))
			end
		end
		_p('')

		-- identify the shell type
		_p('SHELLTYPE := msdos')
		_p('ifeq (,$(ComSpec)$(COMSPEC))')
		_p('  SHELLTYPE := posix')
		_p('endif')
		_p('ifeq (/bin,$(findstring /bin,$(SHELL)))')
		_p('  SHELLTYPE := posix')
		_p('endif')
		_p('')

		-- main build rule(s)
		_p('.PHONY: clean prebuild prelink')
		_p('')

		if os.is("MacOSX") and prj.kind == "WindowedApp" then
			_p('all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET) $(dir $(TARGETDIR))PkgInfo $(dir $(TARGETDIR))Info.plist')
		else
			_p('all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)')
		end
		_p('\t@:')
		_p('')

		-- target build rule
		_p('$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)')
		_p('\t@%s "%s""%s" Linking $(notdir $@)"%s"',
		   echo_cmd, link_color, prj_tag, reset_color)
		_p('\t$(SILENT) $(LINKCMD)')
		_p('\t$(POSTBUILDCMDS)')
		_p('')

		-- Create destination directories. Can't use $@ for this because it loses the
		-- escaping, causing issues with spaces and parenthesis
		_p('$(TARGETDIR):')
		premake.make_mkdirrule("$(TARGETDIR)")

		_p('$(OBJDIR):')
		premake.make_mkdirrule("$(OBJDIR)")

		-- Mac OS X specific targets
		if os.is("MacOSX") and prj.kind == "WindowedApp" then
			_p('$(dir $(TARGETDIR))PkgInfo:')
			_p('$(dir $(TARGETDIR))Info.plist:')
			_p('')
		end

		-- clean target
		_p('clean:')
		_p('\t@echo "%s" Cleaning', prj_tag)
		_p('ifeq (posix,$(SHELLTYPE))')
		_p('\t$(SILENT) rm -f  $(TARGET)')
		_p('\t$(SILENT) rm -rf $(OBJDIR)')
		_p('else')
		_p('\t$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))')
		_p('\t$(SILENT) if exist $(subst /,\\\\,$(OBJDIR)) rmdir /s /q $(subst /,\\\\,$(OBJDIR))')
		_p('endif')
		_p('')

		-- custom build step targets
		_p('prebuild:')
		_p('\t$(PREBUILDCMDS)')
		_p('')

		_p('prelink:')
		_p('\t$(PRELINKCMDS)')
		_p('')

		-- precompiler header rule
		_.pchrules(prj)

		-- per-file rules
		for _, file in ipairs(prj.files) do
			if path.iscppfile(file) then
				_p('$(OBJDIR)/%s.o: %s', _MAKE.esc(path.getbasename(file)), _MAKE.esc(file))
    		_p('\t@%s "%s""%s" Compiling $(notdir $<)"%s"',
    		   echo_cmd, compile_color, prj_tag, reset_color)
--				_p('\t@echo "%s" Compiling $(notdir $<)', prj_tag)
				if (path.iscfile(file)) then
					_p('\t$(SILENT) $(CC) $(CFLAGS) -o "$@" -c "$<"')
				else
					_p('\t$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"')
				end
			elseif (path.getextension(file) == ".rc") then
				_p('$(OBJDIR)/%s.res: %s', _MAKE.esc(path.getbasename(file)), _MAKE.esc(file))
    		_p('\t@%s "%s""%s" Processing $(notdir $<)"%s"',
    		   echo_cmd, compile_color, prj_tag, reset_color)
--				_p('\t@echo "%s" Processing $(notdir $<)', prj_tag)
				_p('\t$(SILENT) windres $< -O coff -o "$@" $(RESFLAGS)')
			end
		end
		_p('')

		-- include the dependencies, built by GCC (with the -MMD flag)
		_p('-include $(OBJECTS:%%.o=%%.d)')

		-- BZFlag customization -- objects depend on their makefiles, makefiles on premake's
		local myname = _MAKE.getmakefilename(prj, true)
		local premake_exec = _MAKE.esc(_PREMAKE_EXEC or 'premake4')
		_p('')
		_p('# targets depend on the build system')
		_p('$(OBJECTS): ' .. myname)
		_p(myname .. ': premake4.lua') -- FIXME -- might be another name
		local topdir = _MAKE.esc(path.getrelative(prj.basedir, prj.solution.basedir))
		_p('\t@%s "%s""%s" Forced a reconfiguration"%s"',
		   echo_cmd, reconfig_color, prj_tag, reset_color)
		_p('\t@(cd "' .. topdir .. '" ; "' .. premake_exec .. '" gmake)')
		_p('')
		_p('.PHONY: reconfig')
		_p('reconfig:')
		_p('\t@(cd "' .. topdir .. '" ; "' .. premake_exec .. '" gmake)')

		-- BZFlag customization -- extra targets for gmake
		if (prj.extratarget) then
			_p('')
			for _, item in ipairs(prj.extratarget) do
				_p('%s: %s', item.target, (item.depends or ''))
				if (item.command) then
					_p('\t' .. item.command)
				end
			end
		end
	end



--
-- Write the makefile header
--

	function premake.gmake_cpp_header(prj, cc, platforms)
		_p('# %s project makefile autogenerated by Premake', premake.action.current().shortname)

		-- set up the environment
		_p('ifndef config')
		_p('  config=%s', _MAKE.esc(premake.getconfigname(prj.solution.configurations[1], platforms[1], true)))
		_p('endif')
		_p('')

		_p('ifndef verbose')
		_p('  SILENT = @')
		_p('endif')
		_p('')

		_p('ifndef CC')
		_p('  CC = %s', cc.cc)
		_p('endif')
		_p('')

		_p('ifndef CXX')
		_p('  CXX = %s', cc.cxx)
		_p('endif')
		_p('')

		_p('ifndef AR')
		_p('  AR = %s', cc.ar)
		_p('endif')
		_p('')
	end


--
-- Write a block of configuration settings.
--

	function premake.gmake_cpp_config(cfg, cc, prj)
		-- prefix each command with prj_tag for multi-job builds
		local prj_tag = get_project_tag(prj)

		_p('ifeq ($(config),%s)', _MAKE.esc(cfg.shortname))

		-- if this platform requires a special compiler or linker, list it now
		local platform = cc.platforms[cfg.platform]
		if platform.cc then
			_p('  CC         = %s', platform.cc)
		end
		if platform.cxx then
			_p('  CXX        = %s', platform.cxx)
		end
		if platform.ar then
			_p('  AR         = %s', platform.ar)
		end

		_p('  OBJDIR     = %s', _MAKE.esc(cfg.objectsdir))
		_p('  TARGETDIR  = %s', _MAKE.esc(cfg.buildtarget.directory))
		_p('  TARGET     = $(TARGETDIR)/%s', _MAKE.esc(cfg.buildtarget.name))
		_p('  DEFINES   += %s', table.concat(cc.getdefines(cfg.defines), " "))
		_p('  INCLUDES  += %s', table.concat(cc.getincludedirs(cfg.includedirs), " "))
		_p('  CPPFLAGS  += %s $(DEFINES) $(INCLUDES)', table.concat(cc.getcppflags(cfg), " "))

		-- set up precompiled headers
		_.pchconfig(cfg)

		_p('  CFLAGS    += $(CPPFLAGS) $(ARCH) %s', table.concat(table.join(cc.getcflags(cfg), cfg.buildoptions), " "))
		_p('  CXXFLAGS  += $(CFLAGS) %s', table.concat(cc.getcxxflags(cfg), " "))
		_p('  LDFLAGS   += %s', table.concat(table.join(cc.getldflags(cfg), cfg.linkoptions, cc.getlibdirflags(cfg)), " "))
		_p('  LIBS      += %s', table.concat(cc.getlinkflags(cfg), " "))
		_p('  RESFLAGS  += $(DEFINES) $(INCLUDES) %s', table.concat(table.join(cc.getdefines(cfg.resdefines), cc.getincludedirs(cfg.resincludedirs), cfg.resoptions), " "))
		_p('  LDDEPS    += %s', table.concat(_MAKE.esc(premake.getlinks(cfg, "siblings", "fullpath")), " "))

		if cfg.kind == "StaticLib" then
			if cfg.platform:startswith("Universal") then
				_p('  LINKCMD    = libtool -o $(TARGET) $(OBJECTS)')
			else
				_p('  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)')
			end
		else
			-- this was $(TARGET) $(LDFLAGS) $(OBJECTS) ... but was having trouble linking to certain
			-- static libraries so $(OBJECTS) was moved up
			_p('  LINKCMD    = $(%s) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)', iif(cfg.language == "C", "CC", "CXX"))
		end

		_p('  define PREBUILDCMDS')
		if #cfg.prebuildcommands > 0 then
			_p('\t@echo "%s" Running pre-build commands', prj_tag)
			_p('\t%s', table.implode(cfg.prebuildcommands, "", "", "\n\t"))
		end
		_p('  endef')

		_p('  define PRELINKCMDS')
		if #cfg.prelinkcommands > 0 then
			_p('\t@echo "%s" Running pre-link commands', prj_tag)
			_p('\t%s', table.implode(cfg.prelinkcommands, "", "", "\n\t"))
		end
		_p('  endef')

		_p('  define POSTBUILDCMDS')
		if #cfg.postbuildcommands > 0 then
			_p('\t@echo "%s" Running post-build commands', prj_tag)
			_p('\t%s', table.implode(cfg.postbuildcommands, "", "", "\n\t"))
		end
		_p('  endef')

--		error('shit')

		_p('endif')
		_p('')
	end


--
-- Precompiled header support
--

	function _.pchconfig(cfg)
		if not cfg.flags.NoPCH and cfg.pchheader then
			_p('  PCH        = %s', _MAKE.esc(path.getrelative(cfg.location, cfg.pchheader)))
			_p('  GCH        = $(OBJDIR)/%s.gch', _MAKE.esc(path.getname(cfg.pchheader)))
			_p('  CPPFLAGS  += -I$(OBJDIR) -include $(OBJDIR)/%s', _MAKE.esc(path.getname(cfg.pchheader)))
		end
	end

	function _.pchrules(prj)
		local prj_tag = get_project_tag(prj)
		_p('ifneq (,$(PCH))')
		_p('$(GCH): $(PCH)')
		_p('\t@echo "%s $(notdir $<)"', prj_tag)
		_p('\t-$(SILENT) cp $< $(OBJDIR)')
		if prj.language == "C" then
			_p('\t$(SILENT) $(CC) $(CFLAGS) -o "$@" -c "$<"')
		else
			_p('\t$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"')
		end
		_p('endif')
		_p('')
	end
