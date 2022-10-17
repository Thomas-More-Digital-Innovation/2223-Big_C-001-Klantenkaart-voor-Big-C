<script>
	import { page } from '$app/stores';

	function goBack() {
		history.back();
	}
</script>

<div class="hero min-h-screen" style="background-image: url(/coffee-beans.jpg);">
	<div class="hero-overlay bg-opacity-60" />
	<div class="hero-content text-center text-neutral-content">
		<div class="max-w-2xl">
			<h1 class="mb-5 text-5xl font-bold">{$page.status} - {$page.error?.message}</h1>
			<p class="mb-5">
				Oh beans! It looks like something went wrong here... Please try again later!
			</p>
			<button class="btn btn-primary" on:click={goBack}>Go back</button>
		</div>
	</div>
</div>
