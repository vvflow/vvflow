make_util = \
	echo "--- $1 ./$2"; \
	$(MAKE) --no-print-directory -C $2 $3

all clean:
	@$(call make_util,"Building",vvcompose,$@)
	@$(call make_util,"Building",vvflow,$@)
	@$(call make_util,"Building",vvplot,$@)
	@$(call make_util,"Building",vvxtract,$@)

install uninstall:
	@$(call make_util,"Installing",vvcompose,$@)
	@$(call make_util,"Installing",vvflow,$@)
	@$(call make_util,"Installing",vvplot,$@)
	@$(call make_util,"Installing",vvxtract,$@)
	@$(call make_util,"Installing",scripts,$@)
