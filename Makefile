TOPTARGETS = build clean

IMAGES = common \
		cpu/code-in-scratchpad \
		dma/otc-test \
		gpu/bandwidth \
		gpu/benchmark \
		gpu/clipping \
		gpu/gp0-e1 \
		gpu/lines \
		gpu/mask-bit \
		gpu/quad \
		gpu/rectangles \
		gpu/texture-overflow \
		gpu/transparency \
		gpu/triangle \
		gpu/vram-to-vram-overlap \
		gte-fuzz \
		spu/test \
		spu/stereo \
		timers

$(TOPTARGETS): $(IMAGES)
$(IMAGES):
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTAGETS) $(IMAGES)
